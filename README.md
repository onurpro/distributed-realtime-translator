> **Status:** Functional prototype. Four-person ENSC 351 course project at SFU, 
> Fall 2025. I built the software stack (network HAL, audio HAL, CMake monorepo, 
> Hub orchestrator, TUI dashboard, and the three AI-engine C++ wrappers); 
> teammates contributed the 3D-printed enclosures and final hardware assembly.

# ENSC 351: Distributed Real-Time Translator (Network Hub)

A distributed, fully offline speech-to-speech translator. English audio in through a USB mic, French audio out through a speaker, no cloud calls. Four BeagleY-AI boards (ARM64 Cortex-A53) on a private gigabit subnet, each running a specialized C/C++ binary, coordinated by a custom TCP hub.

The architecture follows a **Hub-and-Spoke** model where a central orchestrator (Board 4) manages data flow between specialized AI inference nodes via **TCP Sockets**.

## Highlights

- **Custom TCP hub-and-spoke protocol** in C, with persistent sockets between a single client orchestrator and three inference servers (`hal/src/network.c`).
- **Cross-compilation from x86 to ARM64** via a single CMake monorepo with `BOARD={1,2,3,4}` logic gates that isolate each binary's dependencies and resolve a `ggml` target-name collision between `whisper.cpp` and `llama.cpp`.
- **Stateless LLM context manager** that tears down and rebuilds the `llama.cpp` KV cache per sentence, fixing a recurring `inconsistent sequence positions` crash and eliminating cross-sentence hallucinations.
- **ncurses TUI dashboard** on the Hub for live multi-node debugging from a single terminal instead of four SSH sessions.
- **Pipelined async pipeline** (bounded queues between STT/translation/TTS) so Board 2 can translate sentence N+1 while Board 3 is still speaking N.

## 🏗 Architecture & Milestones

The system is modularized into four independent nodes. All nodes now operate as network servers/clients.

| Board | Role | Engine | Port | Status |
| :--- | :--- | :--- | :--- | :--- |
| **Board 1** | **The Ear** (Speech-to-Text) | [Whisper.cpp](https://github.com/ggerganov/whisper.cpp) | `8001` | ✅ TCP Server |
| **Board 2** | **The Brain** (Translation) | [Llama.cpp](https://github.com/ggerganov/llama.cpp) | `8002` | ✅ TCP Server |
| **Board 3** | **The Mouth** (Text-to-Speech) | [Piper](https://github.com/rhasspy/piper) | `8003` | ✅ TCP Server |
| **Board 4** | **The Hub** (Orchestrator) | Custom Logic | N/A | ✅ Client/Manager |

## 📂 Project Structure

```text
.
├── app/                    # Application Source Code
│   ├── src/                # Main server logic (main_stt.c, main_hub.c, etc.)
│   └── include/            # C++ Managers (STT, Translator, TTS)
├── hal/                    # Hardware Abstraction Layer
│   ├── src/audio_hal.c     # Unified Audio (ALSA recording & playback)
│   └── src/network.c       # Socket networking (Server/Client implementation)
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
    sudo apt install cmake build-essential alsa-utils libasound2-dev git libncurses-dev:arm64


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

      ```board1-stt``` (Whisper Server)

      ```board2-trans``` (Translation Server)

      ```board3-tts``` (TTS Server)

      ```board4-hub``` (The Network Hub)

  5. Click Build on the bottom bar by the gear.

**Option B: Command Line**

You can build directly from the terminal using the preset names:

    # Build All Components
    cmake --preset board1-stt && cmake --build --preset board1-stt
    cmake --preset board2-trans && cmake --build --preset board2-trans
    cmake --preset board3-tts && cmake --build --preset board3-tts
    cmake --preset board4-hub && cmake --build --preset board4-hub

*Executables are output to: ```build/app/boardX_name``` and deployed to ```~/ensc351/public/final/boardX_name``` via NFS.*

## 💻 Usage: Full System Simulation

To simulate the distributed system on a single board (Localhost Cluster), you must run 4 separate terminal sessions.

**Terminal 1: Start The Ear (STT)**

    ./board1_stt
    # Output: Server listening on port 8001...

**Terminal 2: Start The Brain (Translator)**

    ./board2_trans
    # Output: Server listening on port 8002...

**Terminal 3: Start The Mouth (TTS)**

    ./board3_tts
    # Output: Server listening on port 8003...

**Terminal 4: Run The Hub** This triggers the pipeline and launches the visual dashboard. Pass a WAV file as an argument.

    ./board4_hub samples/jfk.wav

**Expected Output (Terminal 4):**

    [HUB] Connecting to Board 1 (Ear)...
    [HUB] Sent: samples/jfk.wav
    [HUB] Received: "Ask not what your country..."
    [HUB] Connecting to Board 2 (Brain)...
    [HUB] Received: "Ne demandez pas ce que votre pays..."
    [HUB] Connecting to Board 3 (Mouth)...
    [HUB] Process Complete.

*(You should hear the French audio play from the speaker connected to the board running Terminal 3)*

## 🧠 Technical Credits

  **Whisper.cpp** by Georgi Gerganov

  **Llama.cpp** by Georgi Gerganov

  **Piper TTS** by Rhasspy

  **TinyLlama-1.1B** (TheBloke Quantization)
