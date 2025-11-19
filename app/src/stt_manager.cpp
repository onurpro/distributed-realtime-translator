#include "stt_manager.h"
#include "whisper.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdio> // For printf

static struct whisper_context * ctx = nullptr;

// --- NEW: Progress Callback Function ---
// This function is called repeatedly by Whisper during transcription.
void progress_callback(struct whisper_context * /*ctx*/, struct whisper_state * /*state*/, int progress, void * /*user_data*/) {
    int bar_width = 50;
    int pos = (progress * bar_width) / 100;

    // \r moves cursor to start of line (to overwrite previous bar)
    printf("\rProcessing: [");
    for (int i = 0; i < bar_width; ++i) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %d%%", progress);
    fflush(stdout); // Force print immediately
}

extern "C" { 

void STT_init(const char* model_path) {
    struct whisper_context_params cparams = whisper_context_default_params();
    ctx = whisper_init_from_file_with_params(model_path, cparams);

    if (ctx == nullptr) {
        fprintf(stderr, "[APP] Failed to initialize whisper context\n");
        return;
    }
    printf("[APP] Whisper initialized.\n");
}

// Helper to read WAV file
std::vector<float> read_wav(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        fprintf(stderr, "[APP] Error: Could not open file %s\n", filename);
        return {};
    }

    file.seekg(44); // Skip WAV header

    std::vector<int16_t> pcm16;
    int16_t sample;
    while (file.read(reinterpret_cast<char*>(&sample), sizeof(sample))) {
        pcm16.push_back(sample);
    }

    std::vector<float> pcmf32(pcm16.size());
    for (size_t i = 0; i < pcm16.size(); ++i) {
        pcmf32[i] = static_cast<float>(pcm16[i]) / 32768.0f;
    }

    return pcmf32;
}

int STT_transcribe_file(const char* wav_path) {
    if (!ctx) return -1;

    printf("[APP] Loading file: %s\n", wav_path);
    std::vector<float> pcmf32 = read_wav(wav_path);
    
    if (pcmf32.empty()) {
        fprintf(stderr, "[APP] Failed to read audio data\n");
        return -1;
    }

    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    
    // --- NEW: Register the progress callback ---
    wparams.progress_callback = progress_callback;
    wparams.progress_callback_user_data = nullptr;

    wparams.n_threads = 4; 
    wparams.print_progress = false; // Disable default messy text output
    wparams.print_special = false;
    wparams.print_realtime = false;
    wparams.language = "en"; 

    // Run the inference
    if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
        fprintf(stderr, "\n[APP] Failed to process audio\n");
        return -1;
    }
    
    // Print a newline to ensure the transcription starts on a fresh line
    printf("\n"); 

    // Output the text
    const int n_segments = whisper_full_n_segments(ctx);
    printf("\n--- TRANSCRIPTION ---\n");
    for (int i = 0; i < n_segments; ++i) {
        const char * text = whisper_full_get_segment_text(ctx, i);
        printf("%s", text);
    }
    printf("\n---------------------\n");

    return 0;
}

void STT_free(void) {
    whisper_free(ctx);
}

} // extern "C"