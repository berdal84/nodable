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

        BaseEvent*                      dispatch( EventID );                // Create and push a basic event to the queue
        void                            dispatch_delayed( EventID, u64_t);  // Does the same as dispatch(EventID) with a delay in millisecond. A delay of 0ms will be processed after a regular dispatch though.
        void                            dispatch( BaseEvent* _event);       // Push an existing event to the queue.
        BaseEvent*                      poll_event();                       // Pop the first event in the queue
        const std::vector<BaseAction*>& get_actions() const;                // Get all the actions bound to any event
        template<typename ActionT>
        void bind( const char* label, Shortcut shortcut = {}, typename ActionT::event_data_t&& event_state = {} )
        {
            static_assert( std::is_base_of_v<BaseAction, ActionT> );
            BaseAction* action = new ActionT(label, shortcut, event_state );
            add_action(action);
        }
        template<typename ActionT>
        void bind( const char* label, Shortcut shortcut = {} )
        {
            static_assert( std::is_base_of_v<BaseAction, ActionT> );
            BaseAction* action = new ActionT(label, shortcut);
            add_action(action);
        }
        void bind( const char* label, EventID event_id, const Shortcut& shortcut = {} )
        {
            auto* action = new BaseAction(event_id, label, shortcut);
            add_action(action);
        }
        template<typename EventT>
        BaseEvent* dispatch(typename EventT::data_t&& state = {} )
        {
            static_assert( std::is_base_of_v<BaseEvent, EventT> );
            auto new_event = new EventT(state);
            dispatch(new_event);
            return new_event;
        }

        const BaseAction*              get_action_by_type(u16_t type);     // Get the action bound to a given event type
        static EventManager&           get_instance();
    private:
        void                           add_action( BaseAction* _action);
        static EventManager*           s_instance;
        std::queue<BaseEvent*>         m_events;
        std::vector<BaseAction*>       m_actions;
        std::map<u16_t, BaseAction*>   m_actions_by_event_type;
    };
}