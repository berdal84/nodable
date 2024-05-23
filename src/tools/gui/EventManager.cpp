#include "EventManager.h"

#include "tools/core/assertions.h"
#include "tools/core/async.h"
#include "tools/core/log.h"
#include "tools/core/reflection/type.h"

using namespace tools;

EventManager* EventManager::s_instance = nullptr;

EventManager::~EventManager()
{
    LOG_VERBOSE("tools::EventManager", "Destructor ...\n");
    s_instance = nullptr;
    LOG_VERBOSE("tools::EventManager", "Destructor " OK "\n");
}
EventManager::EventManager()
{
    LOG_VERBOSE("tools::EventManager", "Constructor ...\n");
    EXPECT(!s_instance, "cannot have two instances at a time");
    s_instance = this;
    LOG_VERBOSE("tools::EventManager", "Constructor " OK "\n");
}

EventManager& EventManager::get_instance()
{
    EXPECT(s_instance, "No instance found.");
    return *s_instance;
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
    async::run_task( [&]() -> void { dispatch(event); }, delay );
}