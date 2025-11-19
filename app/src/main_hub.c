#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hal/network.h"
#include "tui_manager.h"

#define IP_STT   "127.0.0.1"
#define PORT_STT 8001
#define IP_TRANS "127.0.0.1"
#define PORT_TRANS 8002
#define IP_TTS   "127.0.0.1"
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

    char full_transcript[BUFFER_SIZE];

    // Step 1: Full Transcription
    TUI_update_ear("Listening (Whisper)...", argv[1]);
    TUI_log("Starting Batch Transcription on Board 1...");
    
    if (send_request("Ear", IP_STT, PORT_STT, argv[1], full_transcript) <= 0) {
        TUI_close();
        printf("Error: Board 1 failed.\n");
        return 1;
    }
    
    int total_len = strlen(full_transcript);
    TUI_update_ear("Received", "Processing text...");
    TUI_log("Transcript received (%d chars)", total_len);

    // Step 2: Chunking Loop
    char* cursor = full_transcript;
    char* start = cursor;
    
    while (*cursor) {
        if (*cursor == '.' || *cursor == '?' || *cursor == '!' || *cursor == '\n') {
            char original = *cursor;
            *cursor = '\0'; 
            
            process_sentence(start);

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
    
    TUI_log("All jobs complete.");
    sleep(3); // Hold display
    TUI_close();
    return 0;
}