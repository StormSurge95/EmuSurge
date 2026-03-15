#include <SDL3/SDL.h>

#include "./systems/NES/NES.h"

//#define DEBUG

const std::string PATH = "C:/roms/NES/Tests/PPU/test_ppu_read_buffer.nes";

static std::array<bool, 8> getButtons(SDL_Gamepad* gp) {
    std::array<bool, 8> buttons = {false, false, false, false, false, false, false, false};

    if (gp) {
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_SOUTH)) {
            buttons[0] = true; // A
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_EAST)) {
            buttons[1] = true; // B
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_BACK)) {
            buttons[2] = true; // Select
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_START)) {
            buttons[3] = true;
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_UP)) {
            buttons[4] = true;
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) {
            buttons[5] = true;
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) {
            buttons[6] = true;
        }
        if (SDL_GetGamepadButton(gp, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) {
            buttons[7] = true;
        }
    }

    return buttons;
}

int main(int argc, int* argv[]) {
    try {
#ifdef DEBUG
        NES* nes = new NES(true);
#else
        NES* nes = new NES();
#endif
        if (!nes->loadCartridge(PATH)) return 1;

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

#ifdef DEBUG
                    std::ifstream ref("C:/Users/Redux/Desktop/Emulator Tests/NES/CPU/nestest.log");
                    if (!ref.is_open())
                        return 2;

                    nes->cpu->debugFile = &ref;
#endif
                    while (!quit) {
                        while (SDL_PollEvent(&e)) {
                            if (e.type == SDL_EVENT_QUIT)
                                quit = true;
                            else if (e.type == SDL_EVENT_GAMEPAD_ADDED) {
                                if (gamepad1 == nullptr) {
                                    gamepad1 = SDL_OpenGamepad(e.gdevice.which);
                                } else {
                                    gamepad2 = SDL_OpenGamepad(e.gdevice.which);
                                }
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

                        if (gamepad1) nes->update(1, getButtons(gamepad1));
                        if (gamepad2) nes->update(2, getButtons(gamepad2));
                        nes->clock();

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
    }
    catch (std::exception& e) {
        printf(e.what());
        return 1;
    }

    return 0;
}