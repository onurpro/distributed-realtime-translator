#include "translator_manager.h"
#include "llama.h"
#include <string>
#include <vector>
#include <cstring>
#include <iostream>

static llama_model* model = nullptr;
static llama_context* ctx = nullptr;
static const llama_vocab* vocab = nullptr; 

extern "C" {

void Translator_init(const char* model_path) {
    llama_backend_init();
    
    llama_model_params model_params = llama_model_default_params();
    model = llama_model_load_from_file(model_path, model_params);

    if (!model) {
        fprintf(stderr, "[APP] Failed to load model: %s\n", model_path);
        return;
    }

    vocab = llama_model_get_vocab(model);

    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048; 
    ctx_params.n_threads = 4; 
    
    ctx = llama_init_from_model(model, ctx_params);
    printf("[APP] Translator Engine Initialized.\n");
}

void batch_add_token(llama_batch &batch, llama_token token, llama_pos pos, bool logits) {
    batch.token[batch.n_tokens] = token;
    batch.pos[batch.n_tokens] = pos;
    batch.n_seq_id[batch.n_tokens] = 1;
    batch.seq_id[batch.n_tokens][0] = 0;
    batch.logits[batch.n_tokens] = logits;
    batch.n_tokens++;
}

// ... (Keep headers and Translator_init) ...

int Translator_process(const char* input, char* output, int max_len) {
    if (!model || !vocab) return -1;

    // 1. Reset Brain (Critical for context clearing)
    if (ctx) llama_free(ctx);
    
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048; 
    ctx_params.n_threads = 4; 
    ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) return -1;

    // 2. Prompt
    std::string prompt = 
        "<|system|>\n"
        "You are a professional translator. Translate the following English text to French. Do not explain. Do not repeat the English.\n"
        "</s>\n"
        "<|user|>\n"
        "Hello world\n"
        "</s>\n"
        "<|assistant|>\n"
        "Bonjour le monde\n"
        "</s>\n"
        "<|user|>\n"
        "Thank you very much for your help.\n"
        "</s>\n"
        "<|assistant|>\n"
        "Merci beaucoup pour votre aide.\n"
        "</s>\n"
        "<|user|>\n"
        "" + std::string(input) + "\n"
        "</s>\n"
        "<|assistant|>\n";

    // --- TOKENIZE ---
    int n_prompt = -llama_tokenize(vocab, prompt.c_str(), prompt.length(), NULL, 0, true, true);
    std::vector<llama_token> tokens_list(n_prompt);
    if (llama_tokenize(vocab, prompt.c_str(), prompt.length(), tokens_list.data(), tokens_list.size(), true, true) < 0) {
        return -1;
    }

    // --- BATCHING ---
    llama_batch batch = llama_batch_init(2048, 0, 1);
    for (size_t i = 0; i < tokens_list.size(); i++) {
        batch_add_token(batch, tokens_list[i], i, false);
    }
    batch.logits[batch.n_tokens - 1] = true;

    if (llama_decode(ctx, batch) != 0) {
        llama_batch_free(batch);
        return -1;
    }

    // --- GENERATION ---
    int n_cur = batch.n_tokens;
    std::string result = "";
    
    auto sparams = llama_sampler_chain_default_params();
    llama_sampler * smpl = llama_sampler_chain_init(sparams);
    llama_sampler_chain_add(smpl, llama_sampler_init_greedy());

    for (int i = 0; i < max_len; i++) {
        llama_token new_token_id = llama_sampler_sample(smpl, ctx, batch.n_tokens - 1);

        if (llama_vocab_is_eog(vocab, new_token_id) || n_cur >= 2048) {
            break;
        }

        char buf[256];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n > 0) {
            std::string piece(buf, n);
            
            if (piece.find("<|") != std::string::npos) break;
            // Strict newline check: Stop if we have meaningful text and hit a newline
            if (piece.find('\n') != std::string::npos && result.length() > 5) break;
            
            if (piece.find('\n') == std::string::npos) {
                result += piece;
                printf("%s", piece.c_str()); 
                fflush(stdout);
            }
        }

        batch.n_tokens = 0;
        batch_add_token(batch, new_token_id, n_cur, true);
        n_cur++;

        if (llama_decode(ctx, batch) != 0) break;
    }
    
    llama_sampler_free(smpl);
    llama_batch_free(batch);

    strncpy(output, result.c_str(), max_len - 1);
    output[max_len - 1] = '\0';

    return 0;
}

void Translator_free(void) {
    if (ctx) llama_free(ctx);
    if (model) llama_model_free(model);
    llama_backend_free();
}

} // extern "C"