#include "Config.h"

static tools::Config* g_conf{nullptr};

tools::Config* tools::init_config()
{
    ASSERT(g_conf == nullptr);
    g_conf = new Config();
    return g_conf;
}

void tools::shutdown_config()
{
    ASSERT(g_conf != nullptr);
    delete g_conf;
    g_conf = nullptr;
}

tools::Config* tools::get_config()
{
    return g_conf;
}

bool tools::has_config()
{
    return g_conf != nullptr;
}