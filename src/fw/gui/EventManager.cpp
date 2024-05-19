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
    async::get_instance().add_task(
        std::async(std::launch::async, [this, event, delay]() -> void {
            std::this_thread::sleep_for(std::chrono::milliseconds{delay});
            dispatch(event);
        })
    );
}