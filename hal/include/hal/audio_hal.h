#ifndef AUDIO_HAL_H
#define AUDIO_HAL_H

#include <stdint.h>

// --- ADD THIS BLOCK ---
#ifdef __cplusplus
extern "C" {
#endif
// ----------------------

// Audio Configuration
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_CHANNELS 1

// --- GENERAL SETUP ---
void Audio_init(void);
void Audio_cleanup(void);

// --- BOARD 1: INPUT ---
int Audio_record(int16_t* buffer, int max_samples, int duration_ms);

// --- BOARD 3: OUTPUT ---
void Audio_play(const char* filename);

// --- ADD THIS BLOCK ---
#ifdef __cplusplus
}
#endif
// ----------------------

#endif