#pragma once
#include "types.h"
#include <functional>

namespace tools
{
    /**
     * Handle asynchronous tasks
     * @example @code
     * using async;
     * init();
     * run_task([]()-> void {...}), 60);
     * ... more tasks ...
     * update(); // clean memory
     * ... more tasks ...
     * shutdown();
     */
    namespace async
    {
        struct Config
        {
            size_t max_capacity{8}; // Maximum task count running in parallel
            size_t reserve_size{1}; // Task count reserved in memory at init
        };

        void init(Config* = nullptr); // Call this before to use.
        void update(); // Basically: clears unused memory
        void shutdown(); // Undo init()
        void run_task(const std::function<void(void)>& function, u64_t delay_in_ms ); // Run a new task with a given delay. update() must be called at regular intervals to ensure memory use does not grow too much
    };
}