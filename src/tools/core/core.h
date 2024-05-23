#pragma once

namespace tools
{
    void core_init();     // Call before to use anything in tools/core
    void core_shutdown(); // Undo core_init()
}