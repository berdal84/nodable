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

        Event*                     dispatch( EventID );                // Create and push a basic event to the queue
        void                       dispatch_delayed( EventID, u64_t);  // Does the same as dispatch(EventID) with a delay in millisecond. A delay of 0ms will be processed after a regular dispatch though.
        void                       dispatch(Event* _event);            // Push an existing event to the queue.
        Event*                     poll_event();                       // Pop the first event in the queue
        const std::vector<Action*>& get_actions() const;               // Get all the actions bound to any event
        template<typename ActionT>
        EventManager& register_action( const char* label, Shortcut shortcut = {}, typename ActionT::payload_t&& payload = {} )
        {
            static_assert( std::is_base_of_v<Action, ActionT> );
            Action* action = new ActionT(label, ActionT::event_id, shortcut, payload);
            add_action(action);
            return *this; // To be able to chain
        }
        EventManager& add_action( const char* label, EventID event_id, const Shortcut& shortcut = {} )
        {
            auto* action = new Action(label, event_id, shortcut);
            add_action(action);
            return *this; // To be able to chain
        }
        template<typename EventT>
        Event* dispatch(const typename EventT::payload_t& payload)
        {
            static_assert( std::is_base_of_v<Event, EventT> );
            auto new_event = new EventT(payload);
            dispatch(new_event);
            return new_event;
        }

        const Action*              get_action_by_type(u16_t type);     // Get the action bound to a given event type

        static EventManager&       get_instance();
    private:
        void                       add_action(Action* _action);

        static EventManager*       s_instance;
        std::queue<Event*>         m_events;
        std::vector<Action*>       m_actions;
        std::map<u16_t, Action*>   m_actions_by_event_type;
    };
}