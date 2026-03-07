#include "emulator_manager.h"

#include <iostream>

int main() {
    EmulatorManager emulator;

    if (!emulator.loadNES("C:/Users/Redux/Desktop/Emulator Tests/NES/CPU/nestest.nes")) {
        std::cout << "Failed to load ROM\n";
        return 1;
    }

    emulator.run();

    return 0;
}