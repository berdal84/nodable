#pragma once

#include <queue>
#include <string>
#include <future>
#include <map>
#include <utility>

#include "tools/core/reflection/FuncType.h"
#include "tools/core/Event.h"
#include "tools/core/types.h"

#include "Action.h"

namespace tools
{
    class ActionManager
    {
    public:
        ~ActionManager();

        const std::vector<IAction*>& get_actions() const; // Get all the actions (bounded and unbounded)
        const IAction*               get_action_with_id(EventID id); // Get the actions bound to a given event id

        template<typename EventT, typename... Args>
        Action<EventT>* new_action(Args&&... args);

        template<class ActionT>
        ActionT* get_action() const;

    private:
        void add_action( IAction* _action);

        std::vector<IAction*>                      m_actions; // all the actions
        std::unordered_multimap<EventID, IAction*> m_actions_by_id; // bound actions only.
    };

    template<typename EventT, typename... Args>
    Action<EventT>* ActionManager::new_action(Args&&... args)
    {
        static_assert(std::is_base_of_v<IEvent, EventT> );
        auto* action = new Action<EventT>(std::forward<Args>(args)...);
        add_action(action);
        return action;
    }

    template<class ActionT>
    ActionT* ActionManager::get_action() const
    {
        static_assert( std::is_base_of_v<IAction, ActionT> );
        auto* action = get_action_with_id( ActionT::event_id );
        return dynamic_cast<ActionT*>( action );
    }

    // Globals to init/get/shutdown the action manager

    [[nodiscard]]
    ActionManager* init_action_manager(); // pointer must be stored to shut it down later
    ActionManager* get_action_manager();
    void           shutdown_action_manager(ActionManager*);
}
