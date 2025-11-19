#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hal/network.h"

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
    
    // 1. Get the FULL transcript first (Board 1 is fast enough to do this)
    if (send_request("Ear", IP_STT, PORT_STT, argv[1], full_transcript) <= 0) {
        printf("[HUB] Failed to get transcript.\n");
        return 1;
    }
    printf("[HUB] Full Transcript Received (%lu bytes).\n", strlen(full_transcript));

    // 2. "Pseudo-Stream" logic: Split by punctuation
    // We iterate through the text and cut it every time we see . ? ! or \n
    char* cursor = full_transcript;
    char* start = cursor;
    
    while (*cursor) {
        // If we hit punctuation, treat it as end of sentence
        if (*cursor == '.' || *cursor == '?' || *cursor == '!' || *cursor == '\n') {
            char original_char = *cursor;
            
            // Terminate string here temporarily to isolate the sentence
            *cursor = '\0'; 
            
            // Process the sentence we just isolated
            process_sentence(start);
            
            // Move start to the character AFTER the punctuation
            start = cursor + 1;
            
            // Skip any whitespace at the start of the NEXT sentence
            while (*start == ' ' || *start == '\n') start++;
        }
        cursor++;
    }

    // Process any remaining text (if the last sentence didn't end with punctuation)
    if (strlen(start) > 1) {
        process_sentence(start);
    }

    printf("\n[HUB] Processing Complete.\n");
    return 0;
}