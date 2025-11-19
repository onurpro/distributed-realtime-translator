#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "stt_manager.h"
#include "hal/network.h"

#define PORT 8001
#define MODEL_PATH "models/ggml-base.en.bin"
#define MAX_TRANSCRIPTION_SIZE 2048

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

        char buffer[1024]; // To hold the filename received

        while (1) {
            // 4. Receive Filename from Board 4
            int len = Network_recv(client_fd, buffer, 1024);
            if (len <= 0) {
                printf("[STT] Client disconnected.\n");
                break;
            }

            printf("[STT] Processing file: '%s'...\n", buffer);
            
            // 5. Run Whisper (Outputs to result_buffer)
            // Note: We assume STT_transcribe_file writes to buffer as discussed.
            // If your current version prints to stdout, we'll adjust stt_manager next.
            if (STT_transcribe_file(buffer, result_buffer, MAX_TRANSCRIPTION_SIZE) == 0) {
                
                // 6. Send Text Back to Board 4
                Network_send(client_fd, result_buffer);
                printf("[STT] Sent response (%lu bytes).\n", strlen(result_buffer));
            } else {
                Network_send(client_fd, "ERROR: Could not transcribe.");
            }
        }
        
        close(client_fd);
    }

    free(result_buffer);
    STT_free();
    return 0;
}