#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hal/network.h"
#include "tui_manager.h"

#define IP_STT   "127.0.0.1"
#define PORT_STT 8001

#define IP_TRANS   "127.0.0.1"
#define PORT_TRANS 8002

#define IP_TTS   "127.0.0.1"
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
    if (strlen(sentence) < 2) return;

    char french_text[4096];
    char tts_ack[128];

    TUI_update_ear("Chunk Complete", sentence);
    
    // Translate
    TUI_update_brain("Processing...", "");
    if (send_request("Brain", IP_TRANS, PORT_TRANS, sentence, french_text) > 0) {
        TUI_update_brain("Done", french_text);
        
        // Speak
        TUI_update_mouth("Speaking...", french_text);
        send_request("Mouth", IP_TTS, PORT_TTS, french_text, tts_ack);
        TUI_update_mouth("Idle", "");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }

    // 1. Start TUI
    TUI_init();

    char full_transcript[4096];

    // Step 1: Get Full Transcript
    TUI_update_ear("Listening...", argv[1]);
    if (send_request("Ear", IP_STT, PORT_STT, argv[1], full_transcript) <= 0) {
        TUI_close();
        printf("Error: Board 1 failed.\n");
        return 1;
    }
    
    int total_len = strlen(full_transcript);
    TUI_update_ear("Received", "Processing text...");

    // Step 2: Chunk & Process
    char* cursor = full_transcript;
    char* start = cursor;
    
    while (*cursor) {
        if (*cursor == '.' || *cursor == '?' || *cursor == '!' || *cursor == '\n') {
            char original = *cursor;
            *cursor = '\0'; 
            
            process_sentence(start);

            // Update Progress
            float p = (float)(cursor - full_transcript) / total_len;
            TUI_update_progress(p);
            
            start = cursor + 1;
            while (*start == ' ' || *start == '\n') start++;
        }
        cursor++;
    }

    if (strlen(start) > 1) {
        process_sentence(start);
        TUI_update_progress(1.0f);
    }

    sleep(2); // Wait so you can see the final state
    TUI_close();
    return 0;
}