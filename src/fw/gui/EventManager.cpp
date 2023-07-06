#include "EventManager.h"
#include <SDL/include/SDL_keyboard.h>
#include "core/log.h"
#include "core/assertions.h"

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

void EventManager::push(EventType _type)
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
    FW_EXPECT(s_instance, "No instance found.");
    return *s_instance;
}
EventManager::~EventManager()
{
    LOG_VERBOSE("fw::EventManager", "Destructor ...\n");
    s_instance = nullptr;
    LOG_VERBOSE("fw::EventManager", "Destructor " OK "\n");
}
EventManager::EventManager()
{
    LOG_VERBOSE("fw::EventManager", "Constructor ...\n");
    FW_EXPECT(!s_instance, "cannot have two instances at a time");
    s_instance = this;
    LOG_VERBOSE("fw::EventManager", "Constructor " OK "\n");
}


std::string Shortcut::to_string() const
{
    std::string result;

    if (mod & KMOD_CTRL) result += "Ctrl + ";
    if (mod & KMOD_ALT)  result += "Alt + ";
    if (key)             result += SDL_GetKeyName(key);
    if (!description.empty()) result += description;

    return result;
}
