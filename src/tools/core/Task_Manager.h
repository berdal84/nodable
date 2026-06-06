#pragma once
#include "Types.h"
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

        void update();
        void schedule_task(const std::function<void(void)>&, u64_t delay_in_ms ); // Run a new task with a given delay. update() must be called at regular intervals to ensure memory use does not grow too much
        void run_task(std::future<void>&& task);

        explicit Task_Manager(const Config& config);
        Config m_conf;
        std::vector<std::future<void>> m_tasks;
    };

    // Globals to init/get/shutdown the task manager

    [[nodiscard]]
    Task_Manager* init_task_manager( const Task_Manager::Config& config = {} ); // Note: make sure you store the ptr since you need it to shut it down.
    Task_Manager* get_task_manager();
    void         shutdown_task_manager(Task_Manager* ); // Undo init_task_manager()
}

