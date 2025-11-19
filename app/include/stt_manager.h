#ifndef STT_MANAGER_H
#define STT_MANAGER_H

// STANDARD C++ GUARD
// This tells the C++ compiler: "Don't mangle these names, they are C functions."
#ifdef __cplusplus
extern "C" {
#endif

void STT_init(const char* model_path);

int STT_transcribe_file(const char* wav_path);

void STT_free(void);

// END GUARD
#ifdef __cplusplus
}
#endif

#endif