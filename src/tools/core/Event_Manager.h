#pragma once

#include <queue>

#include "bdc/Types.hpp"
#include "Event.h"

namespace tools
{
    struct Event_Manager
    {
        std::queue<Event> event_queue;
    };

    Event_Manager*  event_manager_init();  // Note: you don't need to store the pointer, but you do have to call event_manager_shutdown()
    void            event_manager_shutdown();
    Event_Manager*  event_manager();
    void            event_manager_push_event( Event );
    void            event_manager_push_delayed_event( Event, u64_t /* delay_in_ms */ );
    Event           event_manager_pop_event();// Pop the first event from the queue
}