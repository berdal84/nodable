#pragma once
#include "Config.h"

namespace tools
{
    extern Config* g_conf; // Globally accessible configuration. Must be initialized with tools::init() before use;
    void gui_init();     // Call before to use tools::gui
    void gui_shutdown(); // Undo gui_init()
}