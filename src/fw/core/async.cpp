#include "async.h"
#include "log.h"

using namespace fw;

async::async()
{
    tasks.reserve(tasks_reserved_size);
}

async& async::get_instance()
{
    static async instance;
    return instance;
}

void async::add_task(std::future<void>&& task)
{
    // try to reuse a memory space by finding the first future ready (aka done)
    auto task_iterator = tasks.cbegin();
    while(task_iterator != tasks.cend())
    {
        if( task_iterator->valid() && task_iterator->wait_until(std::chrono::system_clock::time_point::min()) == std::future_status::ready )
        {
            tasks.emplace(task_iterator, std::move(task));
            return;
        }
        ++task_iterator;
    }

    // Feedback in the console when max capacity is reached
    if(tasks.size() == tasks.capacity())
    {
        LOG_WARNING("async", "The task vector is full (capacity %zu). It will be resized by the std ...", tasks.capacity());
    }

    // if no space is available, we simply push it back
    tasks.push_back(std::move(task));
}

void async::clean_tasks()
{
    auto task_iterator = tasks.cbegin();
    while(task_iterator != tasks.cend())
    {
        if(task_iterator->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            task_iterator = tasks.erase(task_iterator);
        }
        else
        {
            ++task_iterator;
        }
    }
}
