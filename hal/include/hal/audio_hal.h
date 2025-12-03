/**
 * @file audio_hal.h
 * @brief Hardware Abstraction Layer for Audio Input/Output.
 *
 * This file provides an interface for audio recording and playback
 * using underlying system commands (arecord and aplay).
 */

#ifndef AUDIO_HAL_H
#define AUDIO_HAL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Default audio sample rate in Hz.
 */
#define AUDIO_SAMPLE_RATE 16000

/**
 * @brief Default number of audio channels (1 for Mono).
 */
#define AUDIO_CHANNELS 1

/**
 * @brief Initialize the Audio HAL.
 *
 * Checks for the presence of required system tools (aplay, arecord).
 */
void Audio_init(void);

/**
 * @brief Cleanup the Audio HAL resources.
 */
void Audio_cleanup(void);

/**
 * @brief Record audio to a buffer.
 *
 * Captures audio from the default input device.
 *
 * @param buffer Pointer to the buffer where audio samples will be stored.
 * @param max_samples Maximum number of samples the buffer can hold.
 * @param duration_ms Duration of the recording in milliseconds.
 * @return The number of samples actually read, or 0 on failure.
 */
int Audio_record(int16_t *buffer, int max_samples, int duration_ms);

/**
 * @brief Play a WAV file.
 *
 * Plays the specified WAV file using the default output device.
 *
 * @param filename Path to the WAV file to play.
 */
void Audio_play(const char *filename);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_HAL_H