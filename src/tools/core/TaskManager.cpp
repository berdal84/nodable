#include "TaskManager.h"
#include "assertions.h"
#include "log.h"
#include <future>
#include <thread>
#include <vector>

using std::chrono::system_clock;
using namespace tools;

static TaskManager* g_task_manager{ nullptr };

TaskManager* tools::get_task_manager()
{
    return g_task_manager;
}

TaskManager* tools::init_task_manager(const TaskManager::Config& config)
{
    EXPECT( config.max_capacity >= config.reserve_size, "[tools::init_task_manager] can't reserve more space than capacity!" )
    g_task_manager = new TaskManager(config);
    return g_task_manager;
}

TaskManager::TaskManager(const TaskManager::Config& config )
: m_conf(config)
{
    m_tasks.reserve(m_conf.reserve_size );
}

void TaskManager::run_task(const std::function<void(void)>& function, u64_t delay_in_ms )
{
    std::chrono::milliseconds d{ delay_in_ms };

    // Create an asynchronous function (task)
    auto task = std::async(std::launch::async, [=]() -> void {
        std::this_thread::sleep_for(d);
        function();
    });

    // Run the task
    run_task(std::move(task));
}

void TaskManager::update()
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

void tools::shutdown_task_manager()
{
    EXPECT(g_task_manager != nullptr, "[tools::shutdown_task_manager] must be initialised. Did you call init_task_manager()?");
    delete g_task_manager;
    g_task_manager = nullptr;
}

void TaskManager::run_task(std::future<void>&& task)
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

    EXPECT( m_tasks.size() < m_conf.max_capacity, "[TaskManager::run_task] m_tasks buffer is full. Did you call update() frequently? Consider increasing max_capacity when calling init_task_manager()");

    m_tasks.push_back(std::move(task));
}
