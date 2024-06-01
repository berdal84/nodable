#pragma once
#include "Pool.h"

namespace tools
{
    struct PoolManager
    {
        struct Config
        {
            Pool::Config default_pool_config{};
        };

        Pool* get_pool(); // Right now there is only 1 Pool, but we should split (1 pool per type)
        std::vector<Pool> pools{};
    };

    [[nodiscard]] PoolManager* init_pool_manager(PoolManager::Config = {}); // Call this before to use get_pool_manager()
    PoolManager* get_pool_manager();
    void shutdown_pool_manager(); // Undo init_task_manager()
}
