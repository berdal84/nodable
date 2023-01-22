#include <fw/imgui/Event.h>

std::queue<fw::Event>               fw::EventManager::s_events;
std::vector<fw::BindedEvent>        fw::BindedEventManager::s_binded_events;
std::map<uint16_t, fw::BindedEvent> fw::BindedEventManager::s_binded_events_by_type;

void fw::EventManager::push_event(fw::Event &_event)
{
    s_events.push(_event);
}

size_t fw::EventManager::poll_event(fw::Event &_event)
{
    size_t count = s_events.size();

    if( count )
    {
        _event = s_events.front();
        s_events.pop();
    }

    return count;
}

void fw::EventManager::push_event(fw::EventType _type)
{
    Event simple_event = { _type };
    push_event(simple_event);
}

void fw::BindedEventManager::bind(const fw::BindedEvent &binded_cmd)
{
    s_binded_events.push_back(binded_cmd);
    s_binded_events_by_type.insert({binded_cmd.event_t, binded_cmd});
}

const fw::BindedEvent &fw::BindedEventManager::get_event(uint16_t type)
{
    return s_binded_events_by_type.at(type);
}
