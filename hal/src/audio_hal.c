/**
 * @file audio_hal.c
 * @brief Implementation of the Audio HAL.
 */

#include "hal/audio_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Helper function to check if a system command exists.
 * @param cmd The command to check.
 * @return 1 if the command exists, 0 otherwise.
 */
static int command_exists(const char *cmd) {
  char check_cmd[64];
  snprintf(check_cmd, sizeof(check_cmd), "which %s > /dev/null 2>&1", cmd);
  return (system(check_cmd) == 0);
}

void Audio_init(void) {
  printf("[HAL] Initializing Audio Subsystem...\n");

  if (!command_exists("aplay")) {
    fprintf(stderr, "[HAL] CRITICAL WARNING: 'aplay' not found. Board 3 "
                    "playback will fail.\n");
    fprintf(stderr, "[HAL] Install it: sudo apt install alsa-utils\n");
  }
  if (!command_exists("arecord")) {
    fprintf(stderr, "[HAL] CRITICAL WARNING: 'arecord' not found. Board 1 "
                    "recording will fail.\n");
  }

  printf("[HAL] Audio HAL ready.\n");
}

void Audio_cleanup(void) { printf("[HAL] Audio Subsystem shutdown.\n"); }

int Audio_record(int16_t *buffer, int max_samples, int duration_ms) {
  const char *tmp_file = "/tmp/hal_rec.wav";

  // Construct the record command
  // -f S16_LE: Signed 16-bit Little Endian
  // -r 16000: Rate 16kHz
  // -c 1: Mono
  // -d: Duration in seconds
  char cmd[256];
  snprintf(cmd, sizeof(cmd), "arecord -f S16_LE -r %d -c %d -d %d -q %s",
           AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, duration_ms / 1000, tmp_file);

  printf("[HAL] Recording %d sec audio clip...\n", duration_ms / 1000);
  int ret = system(cmd);
  if (ret != 0) {
    fprintf(stderr, "[HAL] Recording failed (Error code: %d)\n", ret);
    return 0;
  }

  // Read the recorded WAV file
  FILE *f = fopen(tmp_file, "rb");
  if (!f) {
    fprintf(stderr, "[HAL] Failed to open recorded file\n");
    return 0;
  }

  // Skip the 44-byte WAV header
  fseek(f, 44, SEEK_SET);

  int samples_read = fread(buffer, sizeof(int16_t), max_samples, f);
  fclose(f);

  // Remove the temporary file
  remove(tmp_file);

  return samples_read;
}

void Audio_play(const char *filename) {
  if (!filename)
    return;

  printf("[HAL] Playing: %s\n", filename);

  char cmd[512];
  // -q: Quiet mode
  snprintf(cmd, sizeof(cmd), "aplay -q %s", filename);

  int ret = system(cmd);
  if (ret != 0) {
    fprintf(stderr, "[HAL] Playback failed. Is the speaker connected?\n");
  }
}