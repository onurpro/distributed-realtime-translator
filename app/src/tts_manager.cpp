#include "tts_manager.h"
#include "hal/audio_hal.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

static std::string piper_exe;
static std::string voice_model;
static const char* output_wav = "/tmp/tts_output.wav";

extern "C" {

void TTS_init(const char* executable_path, const char* model_path) {
    piper_exe = executable_path;
    voice_model = model_path;
    printf("[APP] TTS Initialized. Engine: %s\n", executable_path);
}

int TTS_speak(const char* text) {
    if (piper_exe.empty() || voice_model.empty()) return -1;

    // 1. Write text to a temporary file to avoid shell injection/quoting issues
    // Use /tmp/ to ensure we have write permissions regardless of CWD
    const char* temp_input = "/tmp/tts_input.txt";
    FILE* f = fopen(temp_input, "w");
    if (!f) {
        perror("[APP] Error: Could not create temp input file"); // Use perror to see the actual error (e.g. Permission denied)
        return -1;
    }
    fprintf(f, "%s", text);
    fclose(f);

    // 2. Construct Command
    // Command format: ./piper -m model.onnx -f output.wav < /tmp/tts_input.txt
    std::string cmd = piper_exe + " --model " + voice_model + 
                      " --length_scale 1.2 " +
                      " --output_file " + output_wav + 
                      " < " + temp_input;

    printf("[APP] Synthesizing: '%s'...\n", text);
    
    // 3. Run Piper
    int ret = system(cmd.c_str());
    if (ret != 0) {
        fprintf(stderr, "[APP] TTS Generation failed (code %d).\n", ret);
        return -1;
    }

    // 4. Play Audio (Using HAL)
    Audio_play(output_wav);

    return 0;
}

void TTS_free(void) {
    // Nothing to clean up for the CLI wrapper
}

} // extern "C"