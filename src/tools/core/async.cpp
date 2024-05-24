#include "async.h"
#include "assertions.h"
#include "log.h"
#include <future>
#include <thread>
#include <vector>

using std::chrono::system_clock;

static tools::TaskManagerConfig g_conf;
static std::vector<std::future<void>> g_tasks;
static bool                           g_is_initialised{false};

void _run_task( std::future<void> && task );

void tools::init_task_manager( TaskManagerConfig* user_config)
{
    if( user_config != nullptr)
        g_conf = *user_config;

    EXPECT( g_conf.max_capacity >= g_conf.reserve_size, "[tools::async] can't reserve more space than capacity!" )

    g_tasks.reserve(g_conf.reserve_size );
    g_is_initialised = true;
}

void tools::run_task(const std::function<void(void)>& function, u64_t delay_in_ms )
{
    EXPECT(g_is_initialised, "[tools::async] must be initialised. Did you call init_task_manager()?");
    std::chrono::milliseconds d{ delay_in_ms };

    // Create an asynchronous function (task)
    auto task = std::async(std::launch::async, [=]() -> void {
        std::this_thread::sleep_for(d);
        function();
    });

    // Run the task
    _run_task(std::move(task));
}

void tools::update_task_manager()
{
    EXPECT(g_is_initialised, "[tools::async] must be initialised. Did you call init_task_manager()?");
    auto task_iterator = g_tasks.cbegin();
    while(task_iterator != g_tasks.cend())
    {
        if(task_iterator->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            task_iterator = g_tasks.erase(task_iterator);
        }
        else
        {
            ++task_iterator;
        }
    }
}

void tools::shutdown_task_manager()
{
    EXPECT(g_is_initialised, "[tools::async] must be initialised. Did you call init_task_manager()?");
    g_is_initialised = false;
    g_tasks.clear();
}

void _run_task(std::future<void>&& task)
{
    // try to reuse a memory space by finding the first future ready (aka done)
    auto it = g_tasks.cbegin();
    while( it != g_tasks.cend())
    {
        auto t = system_clock::time_point::min();
        if( it->valid() && it->wait_until(t) == std::future_status::ready )
        {
            g_tasks.emplace( it, std::move(task));
            return;
        }
        ++it;
    }

    EXPECT( g_tasks.size() < g_conf.max_capacity, "[tools::async] g_tasks buffer is full. Did you call update() frequently? Consider increasing max_capacity when calling init()");

    g_tasks.push_back(std::move(task));
}
