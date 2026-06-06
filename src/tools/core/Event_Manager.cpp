#include "Event_Manager.h"
#include "Asserts.h"
#include "Task_Manager.h"

using namespace tools;

Event_Manager* g_event_manager = nullptr;

Event_Manager* tools::init_event_manager()
{
    VERIFY(g_event_manager == nullptr, "Cannot be called twice"); // singleton
    g_event_manager = new Event_Manager();
    return g_event_manager;
}

Event_Manager* tools::get_event_manager()
{
    VERIFY(g_event_manager != nullptr, "event manager can't be found. Did you call init_ex ?");
    return g_event_manager;
}

void  tools::shutdown_event_manager(Event_Manager* _event_manager)
{
    ASSERT(_event_manager == g_event_manager);  // singleton
    ASSERT(g_event_manager != nullptr);
    delete g_event_manager;
    g_event_manager = nullptr;
}

void Event_Manager::dispatch(IEvent* _event)
{
    m_events.push(_event);
}

IEvent* Event_Manager::poll_event()
{
    if ( m_events.empty() )
    {
        return nullptr;
    }

    IEvent* next_event = m_events.front();
    m_events.pop();
    return next_event;
}

IEvent* Event_Manager::dispatch( Event_ID _event_id )
{
    auto new_event = new IEvent{ _event_id };
    dispatch(new_event );
    return new_event;
}

void Event_Manager::dispatch_delayed(u64_t delay, IEvent* event)
{
    get_task_manager()->schedule_task([this, event]() -> void { this->dispatch(event); }, delay);
}