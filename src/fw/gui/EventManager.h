#pragma once

#include <queue>
#include <string>
#include <future>
#include <map>
#include <utility>

#include "Action.h"
#include "Event.h"
#include "core/reflection/func_type.h"
#include "core/types.h"

namespace fw
{
    class EventManager
    {
    public:
        EventManager();
        EventManager(const EventManager&) = delete;
        ~EventManager();

        IEvent*                      dispatch(EventID);                  // Create and push a basic event to the queue
        void                         dispatch(IEvent* _event);           // Push an existing event to the queue.
        void                         dispatch_delayed(u64_t, IEvent* );  // Does the same as dispatch(Event*) with a delay in millisecond. A delay of 0ms will be processed after a regular dispatch though.
        IEvent*                      poll_event();                       // Pop the first event in the queue
        const std::vector<IAction*>& get_actions() const;                // Get all the actions bound to any event

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

        template<typename ActionT, typename ...Args>
        void bind(Args... args)
        {
            static_assert(std::is_base_of_v<IAction, ActionT> );
            IAction* action = new ActionT(args...);
            add_action(action);
        }

        void bind(
                const char* label,
                EventID event_id,
                const Shortcut& shortcut = {},
                u64_t custom_data = {}
        ){
            auto* action = new IAction(event_id, label, shortcut, custom_data);
            add_action(action);
        }
        template<typename EventT>
        IEvent* dispatch(typename EventT::data_t&& state = {} )
        {
            static_assert( std::is_base_of_v<IEvent, EventT> );
            auto new_event = new EventT(state);
            dispatch(new_event);
            return new_event;
        }

        const IAction*              get_action_by_event_id(EventID id); // Get the action bound to a given event type
        static EventManager&        get_instance();
    private:
        void                        add_action( IAction* _action);
        static EventManager*        s_instance;
        std::queue<IEvent*>         m_events;
        std::vector<IAction*>       m_actions;
        std::map<u16_t, IAction*>   m_actions_by_event_type;
    };
}