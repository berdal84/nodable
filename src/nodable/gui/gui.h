#pragma once
#include "Config.h"

namespace ndbl
{
    extern Config* g_conf; // Globally accessible configuration. Must be initialized with fw::init() before use;
    void init(); // create a new g_conf
    void shutdown(); // delete current g_conf
}