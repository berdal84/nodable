#include "core.h"
#include "tools/core/memory/Pool.h"

void tools::core_init()
{
    LOG_MESSAGE("tools", "core_init() ...\n");
    Pool::init();
    LOG_MESSAGE("tools", "core_init() OK\n");
};

void tools::core_shutdown()
{
    LOG_MESSAGE("tools", "core_shutdown() ...\n");
    Pool::shutdown();
    LOG_MESSAGE("tools", "core_shutdown() OK\n");
};