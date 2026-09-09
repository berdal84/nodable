#include "Event_Manager.h"
#include <queue>
#include "bdc/Types.hpp"
#include "ndbl/core/Asserts.h"
#include "ndbl/core/Event.h"
#include "ndbl/core/Task_Manager.h"

#define VERIFY_EVENT_MANAGER_IS_INITIALIZED() VERIFY( g_event_manager != nullptr, "g_event_manager is not initialized, did you cann event_manager_init() ?")

namespace ndbl
{

static Event_Manager* g_event_manager = nullptr;

Event_Manager* event_manager_init()
{
    VERIFY(g_event_manager == nullptr, "Cannot be called twice"); // singleton
    g_event_manager = bdc::memory_new<Event_Manager>();
    return g_event_manager;
}

Event_Manager* event_manager()
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    return g_event_manager;
}

void  event_manager_shutdown()
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    bdc::memory_delete(g_event_manager);
    g_event_manager = nullptr;
}

void event_manager_push_event(Event _event)
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    g_event_manager->event_queue.push(_event);
}

Event event_manager_pop_event()
{
    VERIFY_EVENT_MANAGER_IS_INITIALIZED();
    if ( g_event_manager->event_queue.empty() )
    {
        return {};
    }

    Event next_event = g_event_manager->event_queue.front();
    g_event_manager->event_queue.pop();
    return next_event;
}

void event_manager_push_delayed_event( Event event, u64_t delay_in_ms)
{
    auto task = [event]() -> void {
        event_manager_push_event(event);
    };
    task_manager_schedule_task(task, delay_in_ms);
}

} // namespace ndbl