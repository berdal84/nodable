#include "nodable/app/Event.h"

using namespace Nodable;

std::queue<Event> EventManager::s_events;

void EventManager::push_event(Event& _event)
{
    s_events.push(_event);
}

size_t EventManager::poll_event(Event& _event)
{
    size_t count = s_events.size();

    if( count )
    {
        _event = s_events.front();
        s_events.pop();
    }

    return count;
}

void EventManager::push_event(EventType _type)
{
    Event simple_event = {
    _type
    };

    push_event(simple_event);
}
