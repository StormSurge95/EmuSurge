#pragma once

#include "../core/emulator_core.h"

#include <functional>
#include <vector>

struct DebugState {
    bool tracingEnabled = false;
    bool breakpointsEnabled = false;

    std::vector<std::function<void(EmulatorCore&)>> debugCallbacks;
};