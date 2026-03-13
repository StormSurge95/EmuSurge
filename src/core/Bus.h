#pragma once

#include <cstdint>
#include <map>
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
            mappings.insert(std::pair<std::string, Mapping>(name, { start, end, device, name }));
        }

        inline uint8_t read(uint16_t addr, bool readonly = false) {
            
            for (std::pair<const std::string, Mapping> &p : mappings) {
                if (addr >= p.second.start && addr <= p.second.end) {
                    return p.second.device->read(addr, readonly);
                }
            }

            return 0;
        }
        inline void write(uint16_t addr, uint8_t data) {
            for (std::pair<const std::string, Mapping> &p : mappings) {
                if (addr >= p.second.start && addr <= p.second.end) {
                    p.second.device->write(addr, data);
                    return;
                }
            }
        }

    protected:
        std::map<std::string, Mapping> mappings;
};