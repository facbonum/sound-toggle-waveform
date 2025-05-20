# Waveform Soundboard

A minimal SDL2-based sound demo that plays a continuous tone when you click the **Play** button. It toggles between a **sine wave** and a **square wave** clicking the **Square (SQ)**, **Triangle (TR)**, and **Noise (NS)** buttons. Useful as a base for building 80s game sound effects, audio experiments, or learning how real-time audio generation works in SDL2.

This project demonstrates:

* Using `SDL_OpenAudioDevice` with a custom callback
* Writing raw audio samples to an output stream
* Generating periodic waveforms mathematically
* Handling input and text rendering in SDL2

It’s a solid base to expand into:

* A full retro game soundboard
* Synthesizer-style audio playback
* MIDI-style waveform instruments

---

## ✨ Features

* Real-time waveform generation using SDL2's audio callback system
* Toggle between sine and square waves
* Play short 440Hz tones on keypress
* Simple SDL2\_ttf text rendering to show waveform type on screen
* Cross-platform: tested on Linux and Windows (MinGW)

---

## 🔧 Controls

| Key            | Action                              |
| -------------- | ----------------------------------- |
| `Play / Stop`  | Play current waveform (440 Hz tone) |
| `SQ / TR / NS` | Toggle between sine/square wave     |
| `Alt + F4`     | Quit the program (Windows)          |

---

## 🛠️ Dependencies

Make sure the following are installed:

* SDL2 development libraries
* SDL2\_ttf (for rendering waveform label text)

---

## 🧪 Build Instructions

### Linux

```bash
g++ sound_toggle_waveform.cpp -o sound_toggle_waveform -lSDL2 -lSDL2_ttf
./sound_toggle_waveform
```

### Windows (MinGW)

Ensure your environment is set up to use `g++` and SDL2 libraries. Then run:

```bash
g++ sound_toggle_waveform.cpp -o sound_toggle_waveform.exe -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
sound_toggle_waveform.exe
```

If you're using MSYS2 or similar, ensure SDL2 and SDL2\_ttf packages are installed via `pacman`.

---

## 🔍 Code Overview

* `AudioCallback` generates the waveform based on a boolean flag.
* The tone is generated only while the play flag is set, with duration capped at 250ms.
* Waveform is toggled by user input and displayed using SDL2\_ttf text rendering.
* All SDL resources are freed properly on exit.

---

## 🚫 Limitations

* Only one frequency (440 Hz) and duration (250 ms) are supported.
* No volume control or envelope shaping.
* Only two waveform types (sine and square).
* No visualizer or waveform preview.

---

## 🧼 Cleanup Notes

Ensure `SDL_Quit()` and `TTF_Quit()` are always called on shutdown to avoid memory leaks. The code also gracefully handles audio device closing and window destruction.

---

## 📜 License

This project is released under the MIT License. Feel free to use it as a base for your own creative or educational projects.
