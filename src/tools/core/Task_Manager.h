#pragma once
#include "bdc/Types.hpp"
#include <functional>
#include <future>
#include <vector>

namespace tools
{
    /**
     * Handle asynchronous tasks
     * @example @code
     * using async;
     * init_task_manager();
     * schedule_task([]()-> void {...}), 60);
     * ... more tasks ...
     * update_task_manager(); // clean memory
     * ... more tasks ...
     * shutdown_task_manager();
     */

    struct Task_Manager
    {
    public:
        struct Config
        {
            size_t max_capacity{8}; // Maximum task count running in parallel
            size_t reserve_size{1}; // Task count reserved in memory at init
        };
        Config                         config;
        std::vector<std::future<void>> tasks;
    };

    // Globals to init/get/shutdown the task manager

    Task_Manager*   task_manager_init( const Task_Manager::Config& config = {} ); 
    Task_Manager*   task_manager();
    void            task_manager_shutdown(); // Undo init_task_manager()
    void            task_manager_update();
    void            task_manager_schedule_task(const std::function<void(void)>&, u64_t delay_in_ms ); // Run a new task with a given delay. update() must be called at regular intervals to ensure memory use does not grow too much
    void            task_manager_run_task(std::future<void>&& task);
}

