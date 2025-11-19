#include "tts_manager.h"
#include "hal/audio_hal.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>

static std::string piper_exe;
static std::string voice_model;
static const char* output_wav = "tts_output.wav";

extern "C" {

void TTS_init(const char* executable_path, const char* model_path) {
    piper_exe = executable_path;
    voice_model = model_path;
    printf("[APP] TTS Initialized. Engine: %s\n", executable_path);
}

int TTS_speak(const char* text) {
    if (piper_exe.empty() || voice_model.empty()) return -1;

    // 1. Construct Command
    // Command format: echo "Text" | ./piper -m model.onnx -f output.wav
    std::string cmd = "echo \"" + std::string(text) + "\" | " + 
                      piper_exe + " --model " + voice_model + 
                      " --length_scale 1.2 " +
                      " --output_file " + output_wav; // + " 2>/dev/null"; // Silence stderr logs

    printf("[APP] Synthesizing: '%s'...\n", text);
    
    // 2. Run Piper
    int ret = system(cmd.c_str());
    if (ret != 0) {
        fprintf(stderr, "[APP] TTS Generation failed.\n");
        return -1;
    }

    // 3. Play Audio (Using HAL)
    Audio_play(output_wav);

    return 0;
}

void TTS_free(void) {
    // Nothing to clean up for the CLI wrapper
}

} // extern "C"