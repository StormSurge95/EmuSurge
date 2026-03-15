#include <SDL3/SDL.h>

#include "./systems/NES/NES.h"

int main(int argc, int* argv[]) {
    NES* nes = new NES(true);
    if (!nes->loadCartridge("C:/roms/NES/Tests/nestest.nes")) return 1;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        return 1;

    SDL_Gamepad* gamepad1 = nullptr;
    SDL_Gamepad* gamepad2 = nullptr;

    const int WIDTH = nes->getScreenWidth();
    const int HEIGHT = nes->getScreenHeight();

    SDL_Window* win = SDL_CreateWindow("EmuSurge", WIDTH * 4, HEIGHT * 4, SDL_WINDOW_RESIZABLE);
    if (win != nullptr) {
        SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
        if (ren != nullptr) {
            SDL_SetDefaultTextureScaleMode(ren, SDL_SCALEMODE_PIXELART);
            SDL_SetRenderLogicalPresentation(ren, WIDTH, HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

            SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
            if (tex != nullptr) {
                SDL_Event e;
                bool quit = false;

                while (!quit) {
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_EVENT_QUIT)
                            quit = true;
                        else if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
                            if (gamepad1 == nullptr) gamepad1 = SDL_OpenGamepad(e.gdevice.which);
                            else gamepad2 = SDL_OpenGamepad(e.gdevice.which);
                        } else if (e.type == SDL_EVENT_GAMEPAD_REMOVED) {
                            if (gamepad1 && SDL_GetGamepadID(gamepad1) == e.gdevice.which) {
                                SDL_CloseGamepad(gamepad1);
                                gamepad1 = gamepad2;
                                gamepad2 = nullptr;
                            } else if (gamepad2 && SDL_GetGamepadID(gamepad2) == e.gdevice.which) {
                                SDL_CloseGamepad(gamepad2);
                                gamepad2 = nullptr;
                            }
                        }
                    }

                    try {
                        nes->clock();
                    }
                    catch (const std::exception& e) {
                        printf(e.what());
                        quit = true;
                    }

                    SDL_UpdateTexture(tex, NULL, nes->getFrameBuffer(), WIDTH * sizeof(uint32_t));

                    SDL_RenderClear(ren);
                    SDL_RenderTexture(ren, tex, NULL, NULL);
                    SDL_RenderPresent(ren);
                }

                SDL_DestroyTexture(tex);
            }
            SDL_DestroyRenderer(ren);
        }
        SDL_DestroyWindow(win);
    }

    return 0;
}

static uint8_t getButtons(SDL_Gamepad* gp) {
    uint8_t buttons = 0;

    if (gp) {
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_SOUTH))
            buttons |= 1 << 0; // A
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_EAST))
            buttons |= 1 << 1; // B
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_BACK))
            buttons |= 1 << 2; // Select
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_START))
            buttons |= 1 << 3;
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_UP))
            buttons |= 1 << 4;
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
            buttons |= 1 << 5;
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
            buttons |= 1 << 6;
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
            buttons |= 1 << 7;
    }

    return buttons;
}