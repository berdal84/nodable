#include "Task_Manager.h"
#include "Asserts.h"
#include <thread> // for std::this_thread::sleep_for

using std::chrono::system_clock;
using namespace tools;

#define VERIFY_TASKMANAGER_IS_INITIALIZED() VERIFY(tools::g_task_manager != nullptr, "Task_Manager is not initialized, did you call task_manager_init() ?")

// private
namespace tools
{
    static Task_Manager* g_task_manager = {};
}

Task_Manager* tools::task_manager()
{
    return g_task_manager;
}

Task_Manager* tools::task_manager_init(const Task_Manager::Config& config)
{
    VERIFY(g_task_manager == nullptr, "Did you initialize twice? OR fogot to shutdown?");
    VERIFY(config.max_capacity >= config.reserve_size, "[tools::init_task_manager] can't reserve more space than capacity!" );
    g_task_manager = bdc::memory_new<Task_Manager>();
    g_task_manager->tasks.reserve(config.reserve_size );
    return g_task_manager;
}

void tools::task_manager_shutdown()
{
    VERIFY_TASKMANAGER_IS_INITIALIZED();
    VERIFY(g_task_manager != nullptr, "Task_Manager must be initialised. Did you call init_task_manager()?");
    delete g_task_manager;
    g_task_manager = nullptr;
}

void tools::task_manager_schedule_task(const std::function<void(void)>& function, u64_t delay_in_ms )
{
    VERIFY_TASKMANAGER_IS_INITIALIZED();
    std::chrono::milliseconds d{ delay_in_ms };

    // Create an asynchronous function (task)
    // TODO: using coroutines would garantee that function() is executed at a given moment in the app loop.
    auto task = std::async(std::launch::async, [=]() -> void {
        std::this_thread::sleep_for(d);
        function();
    });

    // Run the task
    task_manager_run_task(std::move(task));
}

void tools::task_manager_update()
{
    VERIFY_TASKMANAGER_IS_INITIALIZED();
    auto task_iterator = g_task_manager->tasks.cbegin();
    while(task_iterator != g_task_manager->tasks.cend())
    {
        if(task_iterator->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            task_iterator = g_task_manager->tasks.erase(task_iterator);
        }
        else
        {
            ++task_iterator;
        }
    }
}

void tools::task_manager_run_task(std::future<void>&& task)
{
    VERIFY_TASKMANAGER_IS_INITIALIZED();

    // try to reuse a memory space by finding the first future ready (aka done)
    auto it = g_task_manager->tasks.cbegin();
    while( it != g_task_manager->tasks.cend())
    {
        auto t = system_clock::time_point::min();
        if( it->valid() && it->wait_until(t) == std::future_status::ready )
        {
            g_task_manager->tasks.emplace( it, std::move(task));
            return;
        }
        ++it;
    }

    VERIFY(g_task_manager->tasks.size() < g_task_manager->config.max_capacity, "[Task_Manager::schedule_task] m_tasks buffer is full. Did you call update_world_matrix() frequently? Consider increasing max_capacity when calling init_task_manager()");

    g_task_manager->tasks.push_back(std::move(task));
}
