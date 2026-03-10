#include <memory>

#include "../core/IConsole.h"
#include "../systems/NES/NES.h"

static const void run() {
    std::unique_ptr<IConsole> console = std::make_unique<NES>();

    bool running = true;
    while (running) {
        console->clock();

        const uint32_t* frame = console->getFrameBuffer();

        // send frame to SDL texture
    }
}