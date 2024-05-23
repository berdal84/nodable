#include "core.h"
#include "async.h"
#include "tools/core/memory/Pool.h"

void tools::core_init()
{
    LOG_MESSAGE("tools", "core_init() ...\n");
    Pool::init();
    async::init();
    LOG_MESSAGE("tools", "core_init() OK\n");
};

void tools::core_shutdown()
{
    LOG_MESSAGE("tools", "core_shutdown() ...\n");
    Pool::shutdown();
    async::shutdown();
    LOG_MESSAGE("tools", "core_shutdown() OK\n");
};