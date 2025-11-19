#!/bin/bash

echo "--- ENSC 351 Project Setup ---"

# 1. Create Directories
mkdir -p models
mkdir -p ext/piper

# 2. Download Whisper Model (Base English)
if [ ! -f "models/ggml-base.en.bin" ]; then
    echo "[1/3] Downloading Whisper Model (tiny)..."
    wget -P models/ https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-tiny.en.bin
else
    echo "[1/3] Whisper model already exists."
fi

# 3. Download TinyLlama (Quantized)
if [ ! -f "models/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf" ]; then
    echo "[2/3] Downloading TinyLlama Model..."
    wget -P models/ https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf
else
    echo "[2/3] TinyLlama model already exists."
fi

# 4. Download Piper & French Voice
echo "[3/3] Setting up Piper TTS..."
cd ext/piper

# Download Binary if missing
if [ ! -f "piper" ]; then
    echo "      Downloading Piper Engine..."
    wget https://github.com/rhasspy/piper/releases/download/v1.2.0/piper_linux_aarch64.tar.gz
    tar -xzf piper_linux_aarch64.tar.gz
    mv piper/* .
    rm -rf piper piper_linux_aarch64.tar.gz
fi

# Download Voice if missing
if [ ! -f "fr_FR-upmc-medium.onnx" ]; then
    echo "      Downloading French Voice..."
    wget https://huggingface.co/rhasspy/piper-voices/resolve/main/fr/fr_FR/upmc/medium/fr_FR-upmc-medium.onnx
    wget https://huggingface.co/rhasspy/piper-voices/resolve/main/fr/fr_FR/upmc/medium/fr_FR-upmc-medium.onnx.json
fi

echo "--- Setup Complete! ---"