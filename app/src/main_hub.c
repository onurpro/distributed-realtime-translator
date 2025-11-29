#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hal/network.h"

#define IP_STT   "192.168.1.102"
#define PORT_STT 8001

#define IP_TRANS   "192.168.1.103"
#define PORT_TRANS 8002

#define IP_TTS   "192.168.1.101"
#define PORT_TTS 8003

#define BUFFER_SIZE 4096

// Helper: Simple network request
int send_request(const char* module, const char* ip, int port, const char* input, char* output) {
    // 1. Connect
    int sock = Network_connect(ip, port);
    if (sock < 0) {
        fprintf(stderr, "[HUB] Error connecting to %s\n", module);
        return -1;
    }
    
    // 2. Send
    Network_send(sock, input);
    
    // 3. Receive
    int len = Network_recv(sock, output, BUFFER_SIZE);
    
    // 4. Close
    Network_close(sock);
    return len;
}

void process_sentence(char* sentence) {
    // Skip empty/short sentences (e.g. just a newline)
    if (strlen(sentence) < 2) return;

    char french_text[BUFFER_SIZE];
    char tts_ack[128];

    printf("\n[HUB] --- Processing Chunk ---\n");
    printf("[HUB] English: '%s'\n", sentence);

    // 1. Translate this specific sentence
    if (send_request("Brain", IP_TRANS, PORT_TRANS, sentence, french_text) > 0) {
        printf("[HUB] French:  '%s'\n", french_text);
        
        // 2. Speak this specific sentence
        send_request("Mouth", IP_TTS, PORT_TTS, french_text, tts_ack);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }

    char full_transcript[BUFFER_SIZE];

    printf("[HUB] Step 1: Listening to Audio (Board 1)...\n");
    
    // 1. Connect to STT
    int stt_sock = Network_connect(IP_STT, PORT_STT);
    if (stt_sock < 0) {
        printf("[HUB] Failed to connect to STT.\n");
        return 1;
    }

    // 2. Send Filename
    Network_send(stt_sock, argv[1]);

    // 3. Stream Results
    char net_buffer[BUFFER_SIZE];
    char line_buffer[BUFFER_SIZE * 2]; // Accumulate data here
    int line_buf_pos = 0;

    while (1) {
        int len = Network_recv(stt_sock, net_buffer, BUFFER_SIZE);
        if (len <= 0) break;

        // Append received data to line_buffer
        for (int i = 0; i < len; i++) {
            char c = net_buffer[i];
            
            if (c == '\n') {
                // Found a complete line
                line_buffer[line_buf_pos] = '\0';
                
                // Process the line
                if (strncmp(line_buffer, "SEGMENT: ", 9) == 0) {
                    char* text = line_buffer + 9;
                    process_sentence(text);
                } else if (strcmp(line_buffer, "DONE") == 0) {
                    printf("[HUB] STT Finished.\n");
                    goto stt_done; // Break out of outer loop
                } else {
                    printf("[HUB] Unknown message from STT: %s\n", line_buffer);
                }
                
                // Reset buffer for next line
                line_buf_pos = 0;
            } else {
                // Append char if space allows
                if (line_buf_pos < (sizeof(line_buffer) - 1)) {
                    line_buffer[line_buf_pos++] = c;
                }
            }
        }
    }
    
    stt_done:
    Network_close(stt_sock);

    printf("\n[HUB] Processing Complete.\n");
    return 0;
}