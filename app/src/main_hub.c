#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hal/network.h"
#include "tui_manager.h"

#define IP_STT   "192.168.1.102"
#define PORT_STT 8001

#define IP_TRANS   "192.168.1.103"
#define PORT_TRANS 8002

#define IP_TTS   "192.168.1.101"
#define PORT_TTS 8003

#define BUFFER_SIZE 4096

// Helper: Network request with TUI logging
int send_request(const char* module, const char* ip, int port, const char* input, char* output) {
    TUI_log("Connecting to %s...", module);
    
    int sock = Network_connect(ip, port);
    if (sock < 0) {
        TUI_log("ERROR: Connect failed to %s", module);
        return -1;
    }
    
    TUI_log("Sending %lu bytes...", strlen(input));
    Network_send(sock, input);
    
    int len = Network_recv(sock, output, BUFFER_SIZE);
    TUI_log("Received %d bytes from %s", len, module);
    
    Network_close(sock);
    return len;
}

void process_sentence(char* sentence) {
    if (strlen(sentence) < 2) return;

    char french_text[BUFFER_SIZE];
    char tts_ack[128];

    TUI_update_ear("Chunk Complete", sentence);
    
    // 1. Translate
    TUI_update_brain("Processing...", "");
    TUI_log(">> Job: Translate Chunk");
    
    if (send_request("Brain", IP_TRANS, PORT_TRANS, sentence, french_text) > 0) {
        TUI_update_brain("Done", french_text);
        TUI_log("Translation success.");
        
        // 2. Speak
        TUI_update_mouth("Speaking...", french_text);
        TUI_log(">> Job: TTS Playback");
        
        send_request("Mouth", IP_TTS, PORT_TTS, french_text, tts_ack);
        
        TUI_update_mouth("Idle", "");
        TUI_log("Playback complete.");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <wav_file>\n", argv[0]);
        return 1;
    }

    TUI_init();
    TUI_log("Hub Initialized.");

    // Step 1: Connect to STT (Streaming logic)
    TUI_update_ear("Listening (Streaming)...", argv[1]);
    TUI_log("Connecting to STT Board...");

    int stt_sock = Network_connect(IP_STT, PORT_STT);
    if (stt_sock < 0) {
        TUI_log("ERROR: Failed to connect to STT.");
        TUI_close();
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
                    TUI_log("Received Segment: %s", text);
                    process_sentence(text);
                } else if (strcmp(line_buffer, "DONE") == 0) {
                    TUI_log("STT Finished.");
                    goto stt_done; // Break out of outer loop
                } else {
                    TUI_log("Unknown message from STT: %s", line_buffer);
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

    TUI_log("All jobs complete.");
    sleep(3); // Hold display
    TUI_close();
    return 0;
}