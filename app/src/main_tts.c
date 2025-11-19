#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tts_manager.h"

// Paths relative to where you run the app
#define PIPER_PATH "ext/piper/piper"
#define VOICE_PATH "ext/piper/fr_FR-upmc-medium.onnx"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s \"Text to speak\"\n", argv[0]);
        return 1;
    }

    // Combine arguments
    char input_buffer[1024] = "";
    for(int i=1; i<argc; i++) {
        strcat(input_buffer, argv[i]);
        strcat(input_buffer, " ");
    }

    printf("--- Board #3: Text to Speech ---\n");

    TTS_init(PIPER_PATH, VOICE_PATH);
    TTS_speak(input_buffer);
    TTS_free();

    return 0;
}