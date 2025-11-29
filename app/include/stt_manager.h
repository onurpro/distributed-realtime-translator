#ifndef STT_MANAGER_H
#define STT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Whisper with the model path
void STT_init(const char* model_path);

// Transcribe a WAV file.
// - wav_path: Path to input file
// - out_buffer: Buffer to store the resulting text (accumulated)
// - buffer_size: Size of out_buffer
// - callback: Optional function pointer called for each new segment
// Returns 0 on success, -1 on error
typedef void (*stt_callback_t)(const char* segment);
int STT_transcribe_file(const char* wav_path, char* out_buffer, int buffer_size, stt_callback_t callback);

void STT_free(void);

#ifdef __cplusplus
}
#endif

#endif