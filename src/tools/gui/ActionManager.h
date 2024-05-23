#pragma once

#include <queue>
#include <string>
#include <future>
#include <map>
#include <utility>

#include "tools/core/reflection/func_type.h"
#include "tools/core/types.h"

#include "Action.h"
#include "Event.h"

namespace tools
{
    class ActionManager
    {
    public:
        ActionManager();
        ActionManager(const ActionManager&) = delete;
        ~ActionManager();

        const std::vector<IAction*>& get_actions() const;                // Get all the actions bound to any event

        template<typename EventT, typename... Args>
        Action<EventT>* new_action(Args&&... args)
        {
            static_assert(std::is_base_of_v<IEvent, EventT> );
            auto* action = new Action<EventT>(std::forward<Args>(args)...);
            add_action(action);
            return action;
        }

        const IAction* get_action_with_id(EventID id); // Get the action bound to a given event type
        static ActionManager&  get_instance();

        template<class ActionT>
        static ActionT* get_action() // Helper to get a given action type from the ActionManager instance
        { return s_instance->get_action<ActionT>(); }

    private:

        template<class ActionT>
        ActionT* _get_action() const
        {
            static_assert( std::is_base_of_v<IAction, ActionT> );
            auto* action = get_action_with_id( ActionT::event_id );
            return dynamic_cast<ActionT*>( action );
        }

        void                        add_action( IAction* _action);
        static ActionManager*       s_instance;
        std::vector<IAction*>       m_actions;
        std::unordered_multimap<EventID, IAction*> m_actions_by_id;
    };
}