#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "translator_manager.h"
#include "hal/network.h"

#define PORT 8002
#define MODEL_PATH "models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"
#define MAX_OUTPUT_LEN 1024

int main() {
    printf("--- Board #2: Translator Server (Port %d) ---\n", PORT);
    
    Translator_init(MODEL_PATH);
    int server_fd = Network_start_server(PORT);
    if (server_fd < 0) return 1;

    while (1) {
        int client_fd = Network_accept_client(server_fd);
        if (client_fd < 0) continue;

        char input_buffer[2048];
        char output_buffer[MAX_OUTPUT_LEN];

        while (1) {
            // Receive English Text
            int len = Network_recv(client_fd, input_buffer, 2048);
            if (len <= 0) break;

            printf("[TRANS] Input: '%s'\n", input_buffer);
            
            // --- FIX: Clear buffer before processing ---
            memset(output_buffer, 0, MAX_OUTPUT_LEN);

            // Perform Translation
            if (Translator_process(input_buffer, output_buffer, MAX_OUTPUT_LEN) == 0) {
                 // Success: Send result
                 Network_send(client_fd, output_buffer);
                 printf("[TRANS] Sent: '%s'\n", output_buffer);
            } else {
                 // Failure: Send error message (so Hub doesn't hang or get old data)
                 const char* error_msg = "[Error: Translation Failed]";
                 Network_send(client_fd, error_msg);
                 printf("[TRANS] Failed to translate.\n");
            }
        }
        
        close(client_fd);
    }

    Translator_free();
    return 0;
}