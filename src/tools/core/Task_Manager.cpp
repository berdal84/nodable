#include "Task_Manager.h"
#include "Asserts.h"
#include <thread> // for std::this_thread::sleep_for

using std::chrono::system_clock;
using namespace tools;

static Task_Manager* g_task_manager{ nullptr };

Task_Manager* tools::get_task_manager()
{
    return g_task_manager;
}

Task_Manager* tools::init_task_manager(const Task_Manager::Config& config)
{
    VERIFY(config.max_capacity >= config.reserve_size, "[tools::init_task_manager] can't reserve more space than capacity!" );
    g_task_manager = new Task_Manager(config);
    return g_task_manager;
}

Task_Manager::Task_Manager(const Task_Manager::Config& config )
: m_conf(config)
{
    m_tasks.reserve(m_conf.reserve_size );
}

void Task_Manager::schedule_task(const std::function<void(void)>& function, u64_t delay_in_ms )
{
    std::chrono::milliseconds d{ delay_in_ms };

    // Create an asynchronous function (task)
    // TODO: using coroutines would garantee that function() is executed at a given moment in the app loop.
    auto task = std::async(std::launch::async, [=]() -> void {
        std::this_thread::sleep_for(d);
        function();
    });

    // Run the task
    run_task(std::move(task));
}

void Task_Manager::update()
{
    auto task_iterator = m_tasks.cbegin();
    while(task_iterator != m_tasks.cend())
    {
        if(task_iterator->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            task_iterator = m_tasks.erase(task_iterator);
        }
        else
        {
            ++task_iterator;
        }
    }
}

void tools::shutdown_task_manager(Task_Manager* task_manager)
{
    ASSERT(task_manager  == g_task_manager);
    VERIFY(g_task_manager != nullptr, "[tools::shutdown_task_manager] must be initialised. Did you call init_task_manager()?");
    delete g_task_manager;
    g_task_manager = nullptr;
}

void Task_Manager::run_task(std::future<void>&& task)
{
    // try to reuse a memory space by finding the first future ready (aka done)
    auto it = m_tasks.cbegin();
    while( it != m_tasks.cend())
    {
        auto t = system_clock::time_point::min();
        if( it->valid() && it->wait_until(t) == std::future_status::ready )
        {
            m_tasks.emplace( it, std::move(task));
            return;
        }
        ++it;
    }

    VERIFY(m_tasks.size() < m_conf.max_capacity, "[Task_Manager::schedule_task] m_tasks buffer is full. Did you call update_world_matrix() frequently? Consider increasing max_capacity when calling init_task_manager()");

    m_tasks.push_back(std::move(task));
}
