#include "EventManager.h"
#include "core/assertions.h"
#include "core/async.h"
#include "core/log.h"
#include "core/reflection/type.h"
#include <SDL/include/SDL_keyboard.h>
#include <future>
#include <thread>

using namespace fw;

EventManager* EventManager::s_instance = nullptr;

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

EventManager& EventManager::get_instance()
{
    FW_EXPECT(s_instance, "No instance found.");
    return *s_instance;
}

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

void EventManager::add_action(Action _action )
{
    m_actions.push_back( _action );
    m_actions_by_event_type.insert({ _action.event_t, _action });
}

const Action& EventManager::get_action_by_type( u16_t type )
{
    return m_actions_by_event_type.at(type);
}

const std::vector<Action>& EventManager::get_actions() const
{
    return m_actions;
}

void EventManager::push_event_delayed(EventType type, u64_t delay)
{
    fw::async::get_instance().add_task(
        std::async(std::launch::async, [this, type, delay]() -> void {
            std::this_thread::sleep_for(std::chrono::milliseconds{delay});
            push_event( type );
        })
    );
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

