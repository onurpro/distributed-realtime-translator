#include <stdio.h>
#include <unistd.h>
#include "tts_manager.h"
#include "hal/network.h"

#define PORT 8003
// Note: Relative paths assume you run from the deployment folder!
#define PIPER_PATH "ext/piper/piper"
#define VOICE_PATH "ext/piper/fr_FR-upmc-medium.onnx"

int main() {
    printf("--- Board #3: TTS Server (Port %d) ---\n", PORT);
    
    TTS_init(PIPER_PATH, VOICE_PATH);
    int server_fd = Network_start_server(PORT);
    if (server_fd < 0) return 1;

    while (1) {
        int client_fd = Network_accept_client(server_fd);
        if (client_fd < 0) continue;

        char buffer[1024];

        while (1) {
            // 1. Receive French Text
            int len = Network_recv(client_fd, buffer, 1024);
            if (len <= 0) break;

            printf("[TTS] Speaking: '%s'\n", buffer);
            
            // 2. Speak (This blocks until audio finishes)
            TTS_speak(buffer);
            
            // 3. Acknowledge completion
            Network_send(client_fd, "DONE");
        }
        
        close(client_fd);
    }

    TTS_free();
    return 0;
}