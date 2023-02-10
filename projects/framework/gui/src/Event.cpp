#include <fw/gui/Event.h>

using namespace fw;

EventManager* EventManager::s_instance = nullptr;

void EventManager::push_event(Event &_event)
{
    m_events.push(_event);
}

size_t EventManager::poll_event(Event &_event)
{
    size_t count = m_events.size();

    if( count )
    {
        _event = m_events.front();
        m_events.pop();
    }

    return count;
}

void EventManager::push_event(EventType _type)
{
    Event simple_event = { _type };
    push_event(simple_event);
}

void EventManager::bind(const BindedEvent &binded_cmd)
{
    m_binded_events.push_back(binded_cmd);
    m_binded_events_by_type.insert({binded_cmd.event_t, binded_cmd});
}

const BindedEvent &EventManager::get_binded(uint16_t type)
{
    return m_binded_events_by_type.at(type);
}
const std::vector<BindedEvent> &EventManager::get_binded_events() const
{
    return m_binded_events;
}
EventManager& EventManager::get_instance()
{
    return *s_instance;
}
EventManager::~EventManager()
{
    s_instance = nullptr;
}
EventManager::EventManager()
{
    assert(!s_instance); // cannot have two instances at a time
    s_instance = this;
}
