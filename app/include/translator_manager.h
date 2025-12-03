/**
 * @file translator_manager.h
 * @brief Translator Manager using Llama.cpp.
 */

#ifndef TRANSLATOR_MANAGER_H
#define TRANSLATOR_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Translator engine.
 *
 * @param model_path Path to the Llama model file.
 */
void Translator_init(const char *model_path);

/**
 * @brief Process (translate) input text.
 *
 * @param input The input text to translate.
 * @param output Buffer to store the translated text.
 * @param max_len Maximum length of the output buffer.
 * @return 0 on success, -1 on error.
 */
int Translator_process(const char *input, char *output, int max_len);

/**
 * @brief Free Translator resources.
 */
void Translator_free(void);

#ifdef __cplusplus
}
#endif

#endif // TRANSLATOR_MANAGER_H