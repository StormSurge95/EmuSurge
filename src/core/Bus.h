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
                    return m.device->read(addr);
                }
            }

            return 0;
        }
        inline void write(uint16_t addr, uint8_t data) {
            for (Mapping &m : mappings) {
                if (addr >= m.start && addr <= m.end) {
                    m.device->write(addr, data);
                    return;
                }
            }
        }

    private:
        std::vector<Mapping> mappings;
};