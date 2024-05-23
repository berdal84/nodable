#include "core.h"
#include "tools/core/core.h"
#include "tools/core/log.h"

void ndbl::core_init()
{
    LOG_MESSAGE("ndbl", "core_init() ...\n");
    tools::core_init();
    LOG_MESSAGE("ndbl", "core_init() OK\n");
};

void ndbl::core_shutdown()
{
    LOG_MESSAGE("ndbl", "core_shutdown() ...\n");
    tools::core_shutdown();
    LOG_MESSAGE("ndbl", "core_shutdown() OK\n");
};