#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Device.h"

struct Mapping {
    uint16_t start;
    uint16_t end;
    Device* device;
    std::string devName;
};

class Bus {
    public:
        inline void map(Device* device, uint16_t start, uint16_t end, std::string name) {
            mappings.push_back({ start, end, device, name });
        }

        inline uint8_t read(uint16_t addr, bool readonly = false) {
            for (Mapping &m : mappings) {
                if (addr >= m.start && addr <= m.end) {
                    //if (m.devName == "CPU") printf("CPU read %04X\n", addr);
                    //else if (m.devName == "CPU RAM") printf("CPU RAM read %04X\n", addr);
                    //else if (m.devName == "PPU") printf("PPU read %04X\n", addr);
                    //if (m.devName == "CART") printf("CART read %04X\n", addr);
                    return m.device->read(addr);
                }
            }

            return 0;
        }
        inline void write(uint16_t addr, uint8_t data) {
            for (Mapping &m : mappings) {
                if (addr >= m.start && addr <= m.end) {
                    //if (m.devName == "CPU") printf("CPU write %04X = %02X\n", addr, data);
                    //else if (m.devName == "CPU RAM") printf("CPU RAM write %04X = %02X\n", addr, data);
                    //else if (m.devName == "PPU") printf("PPU write %04X = %02X\n", addr, data);
                    m.device->write(addr, data);
                    return;
                }
            }
        }

    private:
        std::vector<Mapping> mappings;
};