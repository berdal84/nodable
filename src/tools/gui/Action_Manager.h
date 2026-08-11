#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "tools/core/Event.h"
#include "tools/gui/Action.h"

namespace tools
{
    struct Action_Manager
    {
        std::vector<Action>                         actions;                    // all the actions
        std::unordered_multimap<Event_Type, size_t> actions_index_by_event_type;  // bound actions only.
    };
 
    [[nodiscard]]
    Action_Manager* action_manager_init(); // pointer must be stored to shut it down later
    void            action_manager_deinit(Action_Manager*);
    Action_Manager* action_manager_get();
    const Action*   action_manager_get_action_with_event_type(const Action_Manager*, Event_Type id); // Get the actions bound to a given event id
    void            action_manager_add_action(Action_Manager*, const Action& _action);
    void            action_manager_add_action(Action_Manager*, const char* label, Event_Type, Shortcut = {});
    void            action_manager_add_action(Action_Manager*, const char* label, Event, Shortcut = {});
    void            action_manager_add_action(Action_Manager*, const char* label, Event_Data__User, Shortcut = {}, u64_t flags = 0 );
}
