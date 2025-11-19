#ifndef TRANSLATOR_MANAGER_H
#define TRANSLATOR_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

void Translator_init(const char* model_path);
int Translator_process(const char* input, char* output, int max_len);
void Translator_free(void);

#ifdef __cplusplus
}
#endif

#endif