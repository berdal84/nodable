#pragma once

#include <cstddef>

#include "bdc/Array.hpp"
#include "bdc/Hash_Map.hpp"
#include "Event.h"
#include "Action.h"

namespace ndbl
{
    struct Action_Manager
    {
        bdc::Inlined_Array<Action, 64>   actions;               // all the actions
        bdc::Hash_Map<u32_t, u32_t>      actions_by_event_type; // Event_Type => position in actions
    };
 
    //
    // Notes:
    //      You can have a single Action_Manager active at a time.
    //      That's why you don't have to pass it as 1st arg on every API call.
    //      action_manager_init() implicitly call action_manager_set
    //

    Action_Manager* action_manager_init(); // can store ptr if needed, but this is probably not required since only 1 manager is enough.
    void            action_manager_shutdown();
    Action_Manager* action_manager();
    void            action_manager_set_current(Action_Manager*);
    const Action*   action_manager_get_action_with_event_type(Event_Type); // Get the actions bound to a given Event_Type
    void            action_manager_register_action(const Action& _action);
}
