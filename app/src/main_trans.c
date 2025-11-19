#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "translator_manager.h"

#define MODEL_PATH "models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s \"Text to translate\"\n", argv[0]);
        return 1;
    }

    // Combine all args into one string (in case user didn't quote input)
    char input_buffer[1024] = "";
    for(int i=1; i<argc; i++) {
        strcat(input_buffer, argv[i]);
        strcat(input_buffer, " ");
    }

    printf("--- Board #2: Translator ---\n");
    printf("Input: '%s'\n", input_buffer);

    Translator_init(MODEL_PATH);

    char output[1024];
    Translator_process(input_buffer, output, 1024);

    printf("\nTranslated: %s\n", output);

    Translator_free();
    return 0;
}