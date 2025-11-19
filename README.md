# ENSC 351: Distributed Real-Time Translator

This project implements a distributed Speech-to-Speech translation system designed for embedded BeagleBoard hardware (Beagley-AI). The system captures English audio, transcribes it to text, translates it to French, and synthesizes spoken French audio.

The architecture follows a **Hub-and-Spoke** model where a central orchestrator (Board 4) manages data flow between specialized AI inference nodes.

## 🏗 Architecture & Milestones

The system is modularized into four independent nodes. Currently, Boards 1, 2, and 3 are fully implemented as standalone modules.

| Board | Role | Engine | Status |
| :--- | :--- | :--- | :--- |
| **Board 1** | **The Ear** (Speech-to-Text) | [Whisper.cpp](https://github.com/ggerganov/whisper.cpp) | ✅ Working (File I/O) |
| **Board 2** | **The Brain** (Translation) | [Llama.cpp](https://github.com/ggerganov/llama.cpp) (TinyLlama) | ✅ Working (Context-aware) |
| **Board 3** | **The Mouth** (Text-to-Speech) | [Piper](https://github.com/rhasspy/piper) | ✅ Working (Neural TTS) |
| **Board 4** | **The Hub** (Network Manager) | Custom Socket Server | 🚧 In Progress |

## 📂 Project Structure

```text
.
├── app/                    # Application Source Code
│   ├── src/                # Main logic for each board
│   └── include/            # C++ Managers (STT, Translator, TTS)
├── hal/                    # Hardware Abstraction Layer
│   ├── src/audio_hal.c     # Unified Audio (ALSA recording & playback)
│   └── src/network.c       # Socket networking (Server/Client)
├── ext/                    # External Libraries (Submodules & Binaries)
│   ├── whisper.cpp/        # STT Inference Engine
│   ├── llama.cpp/          # LLM Inference Engine
│   └── piper/              # TTS Engine (Downloaded via setup.sh)
├── models/                 # Large AI Models (Ignored by Git)
├── CMakeLists.txt          # Master Build Configuration
├── CMakePresets.json       # Build Presets for VS Code
└── setup.sh                # Setup script for models & dependencies

```

## 🛠 Prerequisites

**Hardware**

    Target: Beagley-AI (Cortex-A53 ARM64).

    Peripherals: USB Microphone, Speaker/Headphones.

**Software**

Install these dependencies on the board:
 
    sudo apt update
    sudo apt install cmake build-essential alsa-utils libasound2-dev git


## 🚀 Installation & Setup

Because the AI models and TTS binaries are too large for Git, you must download them manually using the provided script.

1. Clone the Repository:
    ```
    git clone https://github.sfu.ca/oya4/ENSC351-Distributed-Translator.git
    cd ENSC351-Distributed-Translator
    git submodule update --init --recursive
    ```

2. Download Models & Engines: Run the helper script to fetch Whisper (tiny.en), TinyLlama (1.1B), and Piper (fr_FR voice).
    ```
    chmod +x setup.sh
    ./setup.sh
    ```

## ⚙️ Building the Project

This project uses CMake Presets to simplify cross-compilation and board selection.

**Option A: Using VS Code (Recommended)**

  1. Open the project in VS Code.

  2. Install the CMake Tools extension.

  3. Click the CMake tab on the left bar, under Project Status and Configure (it may say [No Preset]) click the pen to select the preset.

  4. Select the board you want to build:

      ```board1-stt``` (Builds Board 1)

      ```board2-trans``` (Builds Board 2)

      ```board3-tts``` (Builds Board 3)

  5. Click Build on the bottom bar by the gear.

**Option B: Command Line**

You can build directly from the terminal using the preset names:

    # Build Board 1 (STT)
    cmake --preset board1-stt
    cmake --build --preset board1-stt

    # Build Board 2 (Translator)
    cmake --preset board2-trans
    cmake --build --preset board2-trans

*Executables are output to: ```build/app/boardX_name``` and sent to ```~/ensc351/public/final/boardX_name``` for easy NFS sharing with the board.*

## 💻 Usage

**Running Board 1 (Speech-to-Text)**

Processes a WAV file and outputs English text.

    # Format: 16kHz, 16-bit Mono WAV
    ./board1_stt samples/jfk.wav

**Running Board 2 (Translator)**

Translates English input to French using the LLM.

    ./board2_trans "Ask not what your country can do for you"

**Running Board 3 (Text-to-Speech)**

Process french and outputs a WAV file and plays it on the speaker.

    ./board3_tts "Bonjour le monde"

## 🧠 Technical Credits

  **Whisper.cpp** by Georgi Gerganov

  **Llama.cpp** by Georgi Gerganov

  **Piper TTS** by Rhasspy

  **TinyLlama-1.1B** (TheBloke Quantization)