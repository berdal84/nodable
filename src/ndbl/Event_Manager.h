#pragma once

#include "bdc/Types.hpp"
#include "bdc/Array.hpp"
#include "Event.h"

namespace ndbl
{
    struct Event_Manager
    {
        u32_t                  event_queue_front; // index to the front element
        u32_t                  event_queue_back;  // index after the next element. Note: is it better than storing event_queue_size?
        Resizable_Array<Event> event_queue_ring;  // array used as a ring buffer
    };

    Event_Manager*  event_manager_init();  // Note: you don't need to store the pointer, but you do have to call event_manager_shutdown()
    void            event_manager_shutdown();
    Event_Manager*  event_manager();
    void            event_manager_push_event( Event );
    void            event_manager_push_delayed_event( Event, u64_t /* delay_in_ms */ );
    const Event&    event_manager_pop_event();// Pop the first event from the queue
}