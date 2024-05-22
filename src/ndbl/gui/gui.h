#pragma once
#include "Config.h"

namespace ndbl
{
    extern Config* g_conf; // Globally accessible configuration. Must be initialized with tools::init() before use;
    void gui_init(); // create a new g_conf
    void gui_shutdown(); // delete current g_conf
}