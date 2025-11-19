#include <stdio.h>
#include <stdlib.h>
#include "stt_manager.h"

// NOTE: Update this to where you put the model on the Beagle
#define MODEL_PATH "models/ggml-tiny.en.bin"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <path_to_wav_file>\n", argv[0]);
        printf("Example: %s test.wav\n", argv[0]);
        return 1;
    }

    const char* wav_file = argv[1];

    printf("--- Board #1: Speech to Text (File Mode) ---\n");

    // 1. Initialize Model
    STT_init(MODEL_PATH);

    // 2. Transcribe the file passed in arguments
    STT_transcribe_file(wav_file);

    // 3. Cleanup
    STT_free();
    
    return 0;
}