/**
 * @file tts_manager.h
 * @brief Text-to-Speech (TTS) Manager.
 */

#ifndef TTS_MANAGER_H
#define TTS_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the TTS engine.
 *
 * @param executable_path Path to the TTS executable (e.g., Piper).
 * @param model_path Path to the TTS model file.
 */
void TTS_init(const char *executable_path, const char *model_path);

/**
 * @brief Convert text to speech and play it immediately.
 *
 * @param text The text to convert to speech.
 * @return 0 on success, -1 on error.
 */
int TTS_speak(const char *text);

/**
 * @brief Free TTS resources.
 */
void TTS_free(void);

#ifdef __cplusplus
}
#endif

#endif // TTS_MANAGER_H