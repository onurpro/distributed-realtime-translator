#ifndef TTS_MANAGER_H
#define TTS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

void TTS_init(const char* executable_path, const char* model_path);

// Convert text to speech and play it immediately
// Returns 0 on success
int TTS_speak(const char* text);

void TTS_free(void);

#ifdef __cplusplus
}
#endif

#endif