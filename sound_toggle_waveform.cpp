// sound_toggle_waveform.cpp
// SDL2 soundboard with channel toggle buttons, bar/waveform visualizer, and frequency slider.

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cmath>

float lastSquare = 0.0f;
float lastTri = 0.0f;
float lastNoise = 0.0f;
uint16_t noiseLFSR = 0xACE1u;

float visSquare = 0.0f;
float visTri = 0.0f;
float visNoise = 0.0f;

const int WAVEFORM_BUFFER_SIZE = 512;
float waveformBuffer[WAVEFORM_BUFFER_SIZE] = {};
int waveformIndex = 0;

bool visualizeWaveform = true;

struct AudioChannel {
    bool enabled;
    float phase;
    float frequency;
    float volume;
};

AudioChannel squareChannel { true, 0.0, 440.0f, 1.0f };
AudioChannel triangleChannel { true, 0.0, 440.0f, 0.7f };
AudioChannel noiseChannel { true, 0.0, 440.0f, 0.5f };

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int SAMPLE_RATE = 44100;
const int AMPLITUDE = 28000;
float globalFrequency = 440;
bool playing = false;

SDL_Rect playButton = { 50, 350, 100, 30 };
SDL_Rect sqButton = { 180, 350, 50, 30 };
SDL_Rect trButton = { 240, 350, 50, 30 };
SDL_Rect nsButton = { 300, 350, 50, 30 };
SDL_Rect freqSliderRect = { 100, 410, 440, 20 };
SDL_Rect toggleVisButton = { 550, 400, 80, 30 };

bool squareEnabled = true;
bool triangleEnabled = true;
bool noiseEnabled = true;

void audioCallback(void* userdata, Uint8* stream, int len) {
    Sint16* buffer = (Sint16*)stream;
    int length = len / 2;

    for (int i = 0; i < length; ++i) {
        float mixedSample = 0.0f;

        if (squareChannel.enabled && playing) {
            float duty = 0.5f;
            float sq = (squareChannel.phase < duty) ? 1.0f : -1.0f;
            lastSquare = sq;
            mixedSample += sq * squareChannel.volume;
            squareChannel.phase += squareChannel.frequency / SAMPLE_RATE;
            if (squareChannel.phase >= 1.0f) squareChannel.phase -= 1.0f;
        }

        if (triangleChannel.enabled && playing) {
            float tri = 2.0f * fabsf(2.0f * triangleChannel.phase - 1.0f) - 1.0f;
            lastTri = tri;
            mixedSample += tri * triangleChannel.volume;
            triangleChannel.phase += triangleChannel.frequency / SAMPLE_RATE;
            if (triangleChannel.phase >= 1.0f) triangleChannel.phase -= 1.0f;
        }

        if (noiseChannel.enabled && playing) {
            bool bit = ((noiseLFSR >> 0) ^ (noiseLFSR >> 1)) & 1;
            noiseLFSR = (noiseLFSR >> 1) | (bit << 14);
            float noise = (noiseLFSR & 1) ? 1.0f : -1.0f;
            lastNoise = noise;
            mixedSample += noise * noiseChannel.volume;
        }

        buffer[i] = static_cast<Sint16>(mixedSample * AMPLITUDE / 3.0f);

        waveformBuffer[waveformIndex] = mixedSample;
        waveformIndex = (waveformIndex + 1) % WAVEFORM_BUFFER_SIZE;
    }
}

void drawVisualizer(SDL_Renderer* renderer, float squareLevel, float triLevel, float noiseLevel) {
    const int baseY = WINDOW_HEIGHT - 100;
    const int width = 20;
    const int spacing = 10;

    SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
    int sqHeight = static_cast<int>(squareLevel * 200);
    SDL_Rect sqBar = {WINDOW_WIDTH - 3 * (width + spacing), baseY - sqHeight, width, sqHeight};
    SDL_RenderFillRect(renderer, &sqBar);

    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
    int triHeight = static_cast<int>(triLevel * 200);
    SDL_Rect triBar = {WINDOW_WIDTH - 2 * (width + spacing), baseY - triHeight, width, triHeight};
    SDL_RenderFillRect(renderer, &triBar);

    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    int noiseHeight = static_cast<int>(noiseLevel * 200);
    SDL_Rect noiseBar = {WINDOW_WIDTH - 1 * (width + spacing), baseY - noiseHeight, width, noiseHeight};
    SDL_RenderFillRect(renderer, &noiseBar);
}

void drawWaveform(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);

    int prevX = 0;
    int prevY = WINDOW_HEIGHT / 2;
    for (int i = 0; i < WAVEFORM_BUFFER_SIZE; ++i) {
        int x = (i * WINDOW_WIDTH) / WAVEFORM_BUFFER_SIZE;
        int y = WINDOW_HEIGHT/2 - static_cast<int>(waveformBuffer[(waveformIndex + i) % WAVEFORM_BUFFER_SIZE] * 100);

        SDL_RenderDrawLine(renderer, prevX, prevY, x, y);
        prevX = x;
        prevY = y;
    }
}

void drawSlider(SDL_Renderer* renderer, SDL_Rect& slider, float frequency) {
    SDL_SetRenderDrawColor(renderer, 170, 0, 0, 255);
    SDL_RenderFillRect(renderer, &slider);

    int markerPos = static_cast<int>((frequency - 100.0f) / 800.0f * slider.w) + slider.x;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, markerPos, slider.y, markerPos, slider.y + slider.h);
}

void drawFreqLabel(SDL_Renderer* renderer, TTF_Font* font) {
    char freqBuf[64];
    snprintf(freqBuf, sizeof(freqBuf), "Freq: %.1f Hz", globalFrequency);

    SDL_Surface* textSurf = TTF_RenderText_Solid(font, freqBuf, {0, 0, 0});
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);

    int tw, th;
    SDL_QueryTexture(textTex, nullptr, nullptr, &tw, &th);
    SDL_Rect textRect = { freqSliderRect.x + (freqSliderRect.w - tw) / 2, freqSliderRect.y - th - 5, tw, th };

    SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
    SDL_FreeSurface(textSurf);
    SDL_DestroyTexture(textTex);
}


int main() {
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    TTF_Init();
    TTF_Font* font = TTF_OpenFont("DejaVuSans.ttf", 16);
    if (!font) {
        std::cerr << "Font load error: " << TTF_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Soundboard", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_AudioSpec want = {};
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 1024;
    want.callback = audioCallback;
    SDL_OpenAudio(&want, nullptr);
    SDL_PauseAudio(0);

    bool running = true;
    SDL_Event e;

    while (running) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 128, 255);
        for (int x = 0; x < WINDOW_WIDTH; x += 32) {
            SDL_RenderDrawLine(renderer, x, 0, x, WINDOW_HEIGHT);
        }
        for (int y = 0; y < WINDOW_HEIGHT; y += 32) {
            SDL_RenderDrawLine(renderer, 0, y, WINDOW_WIDTH, y);
        }
    

        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point mousePoint = { mouseX, mouseY };

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                SDL_Point point = { e.button.x, e.button.y };
                if (SDL_PointInRect(&point, &playButton)) {
                    playing = !playing;
                } else if (SDL_PointInRect(&point, &sqButton)) {
                    squareEnabled = !squareEnabled;
                } else if (SDL_PointInRect(&point, &trButton)) {
                    triangleEnabled = !triangleEnabled;
                } else if (SDL_PointInRect(&point, &nsButton)) {
                    noiseEnabled = !noiseEnabled;
                } else if (SDL_PointInRect(&point, &toggleVisButton)) {
                    visualizeWaveform = !visualizeWaveform;
                }
                squareChannel.enabled = squareEnabled;
                triangleChannel.enabled = triangleEnabled;
                noiseChannel.enabled = noiseEnabled;
            }

            if (e.type == SDL_MOUSEMOTION && e.motion.state & SDL_BUTTON_LMASK) {
                SDL_Point point = { e.motion.x, e.motion.y };
                if (SDL_PointInRect(&point, &freqSliderRect)) {
                    globalFrequency = 100 + (static_cast<float>(e.motion.x) - freqSliderRect.x) / freqSliderRect.w * 800.0f;
                    squareChannel.frequency = globalFrequency;
                    triangleChannel.frequency = globalFrequency;
                    noiseChannel.frequency = globalFrequency;
                }
            }
        }

        float decayRate = 0.01f;
        if (!playing) {
            visSquare = std::max(0.0f, visSquare - decayRate);
            visTri = std::max(0.0f, visTri - decayRate);
            visNoise = std::max(0.0f, visNoise - decayRate);
        } else {
            visSquare = fabs(lastSquare) * squareChannel.volume;
            visTri = fabs(lastTri) * triangleChannel.volume;
            visNoise = fabs(lastNoise) * noiseChannel.volume;
        }

        if (visualizeWaveform) {
            drawWaveform(renderer);
        } else {
            drawVisualizer(renderer, visSquare, visTri, visNoise);
        }

        drawSlider(renderer, freqSliderRect, globalFrequency);

        auto drawButton = [&](SDL_Rect rect, const char* label) {
            bool hovered = SDL_PointInRect(&mousePoint, &rect);
            SDL_Rect drawRect = rect;

            if (hovered) {
                drawRect.x -= 2;
                drawRect.y -= 2;
                drawRect.w += 4;
                drawRect.h += 4;
                SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
            }
            SDL_RenderFillRect(renderer, &drawRect);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &drawRect);

            SDL_Color textColor = hovered ? SDL_Color{255, 255, 255} : SDL_Color{0, 0, 0};
            SDL_Surface* textSurf = TTF_RenderText_Solid(font, label, textColor);
            SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurf);
            int tw, th;
            SDL_QueryTexture(textTex, nullptr, nullptr, &tw, &th);
            SDL_Rect textRect = {drawRect.x + (drawRect.w - tw) / 2, drawRect.y + (drawRect.h - th) / 2, tw, th};
            SDL_RenderCopy(renderer, textTex, nullptr, &textRect);
            SDL_FreeSurface(textSurf);
            SDL_DestroyTexture(textTex);
        };

        drawButton(playButton, playing ? "Stop" : "Play");
        drawButton(sqButton, squareEnabled ? "SQ [*]" : "SQ");
        drawButton(trButton, triangleEnabled ? "TR [*]" : "TR");
        drawButton(nsButton, noiseEnabled ? "NS [*]" : "NS");
        drawButton(toggleVisButton, visualizeWaveform ? "Waveform" : "Bars");
        drawFreqLabel(renderer, font);  // Draw frequency label
        drawSlider(renderer, freqSliderRect, globalFrequency);


        SDL_RenderPresent(renderer);
    }

    SDL_CloseAudio();
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}