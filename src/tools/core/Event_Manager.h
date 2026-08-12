#pragma once

#include <queue>

#include "Types.h"
#include "Event.h"

namespace tools
{
    struct Event_Manager
    {
        std::queue<Event> m_events;
    };

    [[nodiscard]]
    Event_Manager*  event_manager_init();  // Note: make sure you store the ptr since you need it to shut it down.
    void            event_manager_shutdown( Event_Manager* );
    Event_Manager*  event_manager_get();
    void            event_manager_dispatch( Event_Manager*, Event );
    void            event_manager_dispatch_delayed( Event_Manager*, Event, u64_t /* delay_in_ms */ );
    Event           event_manager_poll_event( Event_Manager* );// Pop the first event from the queue
}