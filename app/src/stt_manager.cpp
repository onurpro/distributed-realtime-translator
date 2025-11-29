#include "stt_manager.h"
#include "whisper.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdio>

static struct whisper_context * ctx = nullptr;

// Optional: Keep the progress bar for server logs, or remove if it's too noisy
// void progress_callback(struct whisper_context * /*ctx*/, struct whisper_state * /*state*/, int progress, void * /*user_data*/) {
//     printf("\r[STT] Processing: %d%%", progress);
//     fflush(stdout);
// }

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

std::vector<float> read_wav(const char* filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        fprintf(stderr, "[APP] Error: Could not open file %s\n", filename);
        return {};
    }
    file.seekg(44); 
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

// Internal callback to bridge Whisper C++ callback to our C callback
void internal_segment_callback(struct whisper_context * ctx, struct whisper_state * /*state*/, int n_new, void * user_data) {
    stt_callback_t user_cb = (stt_callback_t)user_data;
    if (!user_cb) return;

    int n_segments = whisper_full_n_segments(ctx);
    int start_segment = n_segments - n_new;

    for (int i = start_segment; i < n_segments; ++i) {
        const char * text = whisper_full_get_segment_text(ctx, i);
        if (text) {
            user_cb(text);
        }
    }
}

int STT_transcribe_file(const char* wav_path, char* out_buffer, int buffer_size, stt_callback_t callback) {
    if (!ctx) return -1;

    // Clear the buffer safely
    if (buffer_size > 0) out_buffer[0] = '\0';

    std::vector<float> pcmf32 = read_wav(wav_path);
    if (pcmf32.empty()) return -1;

    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.n_threads = 4; 
    wparams.print_progress = false;
    wparams.print_special = false;
    wparams.print_realtime = false;
    wparams.language = "en"; 
    
    // Set up the callback
    if (callback) {
        wparams.new_segment_callback = internal_segment_callback;
        wparams.new_segment_callback_user_data = (void*)callback;
    }

    if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
        fprintf(stderr, "\n[APP] Failed to process audio\n");
        return -1;
    }
    printf("\n"); 

    // Accumulate text into buffer (Final full text)
    const int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
        const char * text = whisper_full_get_segment_text(ctx, i);
        
        int current_len = strlen(out_buffer);
        int remaining = buffer_size - current_len - 1; 
        
        if (remaining > 0) {
            strncat(out_buffer, text, remaining);
        }
    }

    return 0;
}

void STT_free(void) {
    whisper_free(ctx);
}

} // extern "C"