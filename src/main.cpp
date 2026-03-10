#include <SDL3/SDL.h>

#include "./systems/NES/NES.h"

int main(int argc, int* argv[]) {
    NES* nes = new NES();
    if (!nes->loadCartridge("C:/roms/NES/Super_Mario_Bros.nes")) return 1;

    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    const int WIDTH = nes->getScreenWidth();
    const int HEIGHT = nes->getScreenHeight();

    SDL_Window* win = SDL_CreateWindow("EmuSurge", WIDTH * 8, HEIGHT * 8, NULL);
    if (win != nullptr) {
        SDL_Renderer* ren = SDL_CreateRenderer(win, NULL);
        if (ren != nullptr) {
            SDL_SetDefaultTextureScaleMode(ren, SDL_SCALEMODE_PIXELART);

            SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB32, SDL_TEXTUREACCESS_STATIC, WIDTH, HEIGHT);
            if (tex != nullptr) {
                SDL_Event e;
                bool quit = false;

                while (!quit) {
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_EVENT_QUIT)
                            quit = true;
                    }

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

    return 0;
}