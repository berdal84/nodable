#include "Pool.h"

using namespace tools;

static PoolManager* g_manager = nullptr;

Pool* tools::get_pool()
{
    ASSERT(g_manager != nullptr)
    return g_manager->get_pool();
}

PoolManager* tools::init_pool_manager(PoolManager::Config cfg)
{
    LOG_VERBOSE("tools", "init_pool_manager ..\n")
    ASSERT(g_manager == nullptr)
    g_manager = new PoolManager();
    Pool pool{cfg.default_pool_config};
    g_manager->pools.emplace_back(pool);
    LOG_VERBOSE("tools", "init_pool_manager OK\n")
    return g_manager;
}

void tools::shutdown_pool_manager()
{
    LOG_VERBOSE("tools", "shutdown_pool_manager ..\n")
    ASSERT(g_manager != nullptr)
    delete g_manager;
    g_manager = nullptr;
    LOG_VERBOSE("tools", "shutdown_pool_manager OK\n")
}

Pool* PoolManager::get_pool()
{
    ASSERT(!pools.empty())
    return &pools.at(0);
}
