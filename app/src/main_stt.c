#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stt_manager.h"
#include "hal/network.h"

#define PORT 8001
#define MODEL_PATH "models/ggml-base.en.bin"
#define MAX_TRANSCRIPTION_SIZE 2048

// Global to allow callback access (Single-threaded server assumption)
static int current_client_fd = -1;

void send_segment_callback(const char* segment) {
    if (current_client_fd < 0) return;
    
    char msg[2048];
    // Simple protocol: "SEGMENT: <text>\n"
    snprintf(msg, sizeof(msg), "SEGMENT: %s\n", segment);
    Network_send(current_client_fd, msg);
    printf("[STT] Sent segment: '%s'\n", segment);
}

int main() {
    printf("--- Board #1: STT Server (Port %d) ---\n", PORT);
    
    // 1. Initialize AI Engine
    STT_init(MODEL_PATH);
    char* result_buffer = (char*)malloc(MAX_TRANSCRIPTION_SIZE);

    // 2. Start Networking
    int server_fd = Network_start_server(PORT);
    if (server_fd < 0) return 1;

    while (1) {
        // 3. Wait for Board 4 to connect
        int client_fd = Network_accept_client(server_fd);
        if (client_fd < 0) continue;
        
        current_client_fd = client_fd;

        char buffer[1024]; // To hold the filename received

        while (1) {
            // 4. Receive Filename from Board 4
            int len = Network_recv(client_fd, buffer, 1024);
            if (len <= 0) {
                printf("[STT] Client disconnected.\n");
                break;
            }

            printf("[STT] Processing file: '%s'...\n", buffer);
            
            // 5. Run Whisper with Streaming Callback
            if (STT_transcribe_file(buffer, result_buffer, MAX_TRANSCRIPTION_SIZE, send_segment_callback) == 0) {
                // 6. Send DONE signal
                Network_send(client_fd, "DONE\n");
                printf("[STT] Finished. Sent DONE.\n");
            } else {
                Network_send(client_fd, "ERROR: Could not transcribe.\n");
            }
        }
        
        close(client_fd);
        current_client_fd = -1;
    }


    free(result_buffer);
    STT_free();
    return 0;
}