#include "EventManager.h"
#include "assertions.h"
#include "log.h"
#include "TaskManager.h"

using namespace tools;

EventManager* g_event_manager = nullptr;

EventManager* tools::init_event_manager()
{
    VERIFY(g_event_manager == nullptr, "Cannot be called twice"); // singleton
    g_event_manager = new EventManager();
    return g_event_manager;
}

EventManager* tools::get_event_manager()
{
    VERIFY(g_event_manager != nullptr, "event manager can't be found. Did you call init_ex ?");
    return g_event_manager;
}

void  tools::shutdown_event_manager(EventManager* _event_manager)
{
    ASSERT(_event_manager == g_event_manager);  // singleton
    ASSERT(g_event_manager != nullptr);
    delete g_event_manager;
    g_event_manager = nullptr;
}

void EventManager::dispatch(IEvent* _event)
{
    m_events.push(_event);
}

IEvent* EventManager::poll_event()
{
    if ( m_events.empty() )
    {
        return nullptr;
    }

    IEvent* next_event = m_events.front();
    m_events.pop();
    return next_event;
}

IEvent* EventManager::dispatch( EventID _event_id )
{
    auto new_event = new IEvent{ _event_id };
    dispatch(new_event );
    return new_event;
}

void EventManager::dispatch_delayed(u64_t delay, IEvent* event)
{
    get_task_manager()->schedule_task([this, event]() -> void { this->dispatch(event); }, delay);
}