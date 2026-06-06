#pragma once

#include <queue>
#include "Types.h"
#include "Event.h"

namespace tools
{
    class Event_Manager
    {
    public:
        IEvent* dispatch(Event_ID);                  // Create and push a basic event to the queue
        void    dispatch(IEvent* _event);           // Push an existing event to the queue.
        void    dispatch_delayed(u64_t, IEvent* );  // Does the same as dispatch(Event*) with a delay in millisecond. A delay of 0ms will be processed after a regular dispatch though.
        IEvent* poll_event();                       // Pop the first event in the queue

        template<typename EventT, typename ...Args>
        void dispatch(Args... args)
        {
            static_assert(std::is_base_of_v<IEvent, EventT> );
            IEvent* event = new EventT(args...);
            dispatch(event);
        }

        template<typename EventT>
        void dispatch_delayed(u64_t delay_in_ms, typename EventT::data_t data)
        {
            static_assert(std::is_base_of_v<IEvent, EventT> );
            IEvent* event = new EventT(data);
            dispatch_delayed(delay_in_ms, event);
        }

        template<typename EventT>
        IEvent* dispatch(typename EventT::data_t&& state = {} )
        {
            static_assert( std::is_base_of_v<IEvent, EventT> );
            auto new_event = new EventT(state);
            dispatch(new_event);
            return new_event;
        }
    private:
        std::queue<IEvent*> m_events;
    };

    // Globals to init/get/shutdown the event manager

    [[nodiscard]]
    Event_Manager* init_event_manager();  // Note: make sure you store the ptr since you need it to shut it down.
    Event_Manager* get_event_manager();
    void          shutdown_event_manager(Event_Manager*);
}