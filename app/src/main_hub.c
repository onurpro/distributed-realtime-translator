/**
 * @file main_hub.c
 * @brief Main entry point for Board 4 (The Hub).
 *
 * Coordinates the distributed translation system by managing
 * STT (Board 1), Translation (Board 2), and TTS (Board 3).
 */

#include "hal/network.h"
#include "tui_manager.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define IP_STT "192.168.1.101"
#define PORT_STT 8001

#define IP_TRANS "192.168.1.102"
#define PORT_TRANS 8002

#define IP_TTS "192.168.1.103"
#define PORT_TTS 8003

#define BUFFER_SIZE 4096

// Thread-safe Queue Implementation
typedef struct Node {
  char *data;
  struct Node *next;
} Node;

typedef struct {
  Node *head, *tail;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int finished; // 1 if no more items will be added
} Queue;

void queue_init(Queue *q) {
  q->head = q->tail = NULL;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->cond, NULL);
  q->finished = 0;
}

void queue_push(Queue *q, const char *str) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->data = strdup(str);
  node->next = NULL;

  pthread_mutex_lock(&q->mutex);
  if (q->tail) {
    q->tail->next = node;
    q->tail = node;
  } else {
    q->head = q->tail = node;
  }
  pthread_cond_signal(&q->cond);
  pthread_mutex_unlock(&q->mutex);
}

// Returns NULL if queue is empty and finished
char *queue_pop(Queue *q) {
  pthread_mutex_lock(&q->mutex);
  while (q->head == NULL && !q->finished) {
    pthread_cond_wait(&q->cond, &q->mutex);
  }

  if (q->head == NULL) {
    pthread_mutex_unlock(&q->mutex);
    return NULL;
  }

  Node *node = q->head;
  char *data = node->data;
  q->head = node->next;
  if (q->head == NULL) {
    q->tail = NULL;
  }
  free(node);
  pthread_mutex_unlock(&q->mutex);
  return data;
}

void queue_finish(Queue *q) {
  pthread_mutex_lock(&q->mutex);
  q->finished = 1;
  pthread_cond_broadcast(&q->cond);
  pthread_mutex_unlock(&q->mutex);
}

// Global Queues
Queue stt_queue; // Segments from STT -> Translator
Queue tts_queue; // Translations -> TTS

// Helper: Network Request
int send_request(const char *module, const char *ip, int port,
                 const char *input, char *output) {
  int sock = Network_connect(ip, port);
  if (sock < 0) {
    TUI_log("ERROR: Connect failed to %s", module);
    return -1;
  }

  Network_send(sock, input);

  int len = Network_recv(sock, output, BUFFER_SIZE);

  Network_close(sock);
  return len;
}

// Worker Threads

void *translator_thread(void *arg) {
  char french_text[BUFFER_SIZE];

  while (1) {
    char *text = queue_pop(&stt_queue);
    if (text == NULL)
      break; // Finished

    TUI_update_brain("Processing...", text);
    TUI_log(">> [Trans] Translating: %.20s...", text);

    if (send_request("Brain", IP_TRANS, PORT_TRANS, text, french_text) > 0) {
      TUI_update_brain("Done", french_text);
      TUI_log(">> [Trans] Success.");

      // Push to TTS queue
      queue_push(&tts_queue, french_text);
    } else {
      TUI_log(">> [Trans] Failed!");
    }

    free(text);
  }

  // Signal TTS that we are done
  queue_finish(&tts_queue);
  return NULL;
}

void *tts_thread(void *arg) {
  char tts_ack[128];

  while (1) {
    char *text = queue_pop(&tts_queue);
    if (text == NULL)
      break; // Finished

    TUI_update_mouth("Speaking...", text);
    TUI_log(">> [TTS] Speaking: %.20s...", text);

    if (send_request("Mouth", IP_TTS, PORT_TTS, text, tts_ack) > 0) {
      TUI_update_mouth("Idle", "");
      TUI_log(">> [TTS] Done.");
    } else {
      TUI_log(">> [TTS] Failed!");
    }

    free(text);
  }
  return NULL;
}

// Main Application Entry

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <wav_file>\n", argv[0]);
    return 1;
  }

  Network_enable_logging(0);
  TUI_init();
  TUI_log("Hub Initialized (Pipelined).");

  // Initialize Queues
  queue_init(&stt_queue);
  queue_init(&tts_queue);

  // Start Threads
  pthread_t trans_tid, tts_tid;
  pthread_create(&trans_tid, NULL, translator_thread, NULL);
  pthread_create(&tts_tid, NULL, tts_thread, NULL);

  // Connect to STT
  TUI_update_ear("Listening...", argv[1]);
  TUI_log("Connecting to STT Board...");

  int stt_sock = Network_connect(IP_STT, PORT_STT);
  if (stt_sock < 0) {
    TUI_log("ERROR: Failed to connect to STT.");
    sleep(5);
    TUI_close();
    return 1;
  }

  // Send Filename
  Network_send(stt_sock, argv[1]);

  // Stream Results
  char net_buffer[BUFFER_SIZE];
  char line_buffer[BUFFER_SIZE * 2];
  int line_buf_pos = 0;

  while (1) {
    int len = Network_recv(stt_sock, net_buffer, BUFFER_SIZE);
    if (len <= 0)
      break;

    for (int i = 0; i < len; i++) {
      char c = net_buffer[i];

      if (c == '\n') {
        line_buffer[line_buf_pos] = '\0';

        if (strncmp(line_buffer, "SEGMENT: ", 9) == 0) {
          char *text = line_buffer + 9;
          TUI_log("Received Segment: %.20s...", text);
          TUI_update_ear("Received Chunk", text);

          // Push to Translation Queue
          queue_push(&stt_queue, text);

        } else if (strcmp(line_buffer, "DONE") == 0) {
          TUI_log("STT Finished.");
          goto stt_done;
        }

        line_buf_pos = 0;
      } else {
        if (line_buf_pos < (sizeof(line_buffer) - 1)) {
          line_buffer[line_buf_pos++] = c;
        }
      }
    }
  }

stt_done:
  Network_close(stt_sock);

  // Signal Translator that STT is done
  queue_finish(&stt_queue);

  // Wait for threads to finish
  pthread_join(trans_tid, NULL);
  pthread_join(tts_tid, NULL);

  TUI_log("All jobs complete.");
  sleep(3);
  TUI_close();
  return 0;
}