#include "Event_Manager.h"
#include <queue>
#include "Asserts.h"
#include "Task_Manager.h"

tools::Event_Manager* g_event_manager = nullptr;

tools::Event_Manager* tools::event_manager_init()
{
    VERIFY(g_event_manager == nullptr, "Cannot be called twice"); // singleton
    g_event_manager = new Event_Manager();
    return g_event_manager;
}

tools::Event_Manager* tools::event_manager_get()
{
    VERIFY(g_event_manager != nullptr, "event manager can't be found. Did you call init_ex ?");
    return g_event_manager;
}

void  tools::event_manager_shutdown(Event_Manager* manager)
{
    ASSERT(manager == g_event_manager);  // singleton
    ASSERT(g_event_manager != nullptr);
    delete g_event_manager;
    g_event_manager = nullptr;
}

void tools::event_manager_dispatch(Event_Manager* manager, Event _event)
{
    manager->m_events.push(_event);
}

tools::Event tools::event_manager_poll_event(Event_Manager* manager)
{
    if ( manager->m_events.empty() )
    {
        return {};
    }

    Event next_event = manager->m_events.front();
    manager->m_events.pop();
    return next_event;
}

void tools::event_manager_dispatch( Event_Manager* manager, Event_Type type )
{
    event_manager_dispatch(manager, Event{ type });
}

void tools::event_manager_dispatch_delayed( Event_Manager* manager, Event event, u64_t delay_in_ms)
{
    get_task_manager()->schedule_task(
        [manager, event]() -> void { event_manager_dispatch(event_manager_get(), event); }
        , delay_in_ms
    );
}