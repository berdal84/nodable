#include "Config.h"
#include "core/Asserts.h"

// private
namespace tools
{
    static Config* g_config = {};
}

tools::Config* tools::config_init()
{
    ASSERT(g_config == nullptr);
    g_config = bdc::memory_new<Config>();
    return g_config;
}

void tools::config_shutdown()
{
    VERIFY(g_config, "tools::Config is not initialized! Did you cann tools::config_init() ? ");
    bdc::memory_delete(g_config);
    g_config = nullptr;
}

tools::Config* tools::config()
{
    VERIFY(g_config, "tools::Config is not initialized! Did you cann tools::config_init() ? ");
    return g_config;
}