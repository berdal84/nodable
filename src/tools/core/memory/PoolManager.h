#pragma once
#if TOOLS_POOL_ENABLE
#include "Pool.h"

namespace tools
{
    struct PoolManager
    {
        struct Config
        {
            Pool::Config default_pool_config{};
        };

        Pool* get_pool(); // Only one Pool for now, but we should be able to have multiple pools (e.g. one per File)
        std::vector<Pool> pools{};
    };

    [[nodiscard]] PoolManager* init_pool_manager(PoolManager::Config = {}); // Call this before to use get_pool_manager()
    PoolManager* get_pool_manager();
    void shutdown_pool_manager(); // Undo init_task_manager()
}
#endif