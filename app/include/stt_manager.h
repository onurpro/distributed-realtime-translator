/**
 * @file stt_manager.h
 * @brief Speech-to-Text (STT) Manager using Whisper.
 */

#ifndef STT_MANAGER_H
#define STT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the STT engine.
 *
 * @param model_path Path to the Whisper model file (e.g.,
 * "models/ggml-base.en.bin").
 */
void STT_init(const char *model_path);

/**
 * @brief Callback function type for receiving transcribed segments.
 * @param segment The transcribed text segment.
 */
typedef void (*stt_callback_t)(const char *segment);

/**
 * @brief Transcribe a WAV file.
 *
 * @param wav_path Path to the input WAV file.
 * @param out_buffer Buffer to store the accumulated transcription.
 * @param buffer_size Size of the output buffer.
 * @param callback Optional callback function invoked for each new segment.
 * @return 0 on success, -1 on error.
 */
int STT_transcribe_file(const char *wav_path, char *out_buffer, int buffer_size,
                        stt_callback_t callback);

/**
 * @brief Free STT resources.
 */
void STT_free(void);

#ifdef __cplusplus
}
#endif

#endif // STT_MANAGER_H