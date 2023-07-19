#pragma once
#include <vector>
#include <future>

namespace fw
{
    /**
     * Handle asynchronous tasks
     * @example @code
     * async::get_instance()
     *       .add_task(std::async(std::launch::async, []()-> void {...}))
     */
    class async
    {
    public:
        async();
        void clean_tasks(); // remove finished tasks
        void add_task(std::future<void>&&);
        static async& get_instance();
    private:
        static constexpr size_t        tasks_reserved_size = 16;
        std::vector<std::future<void>> tasks;
    };
}