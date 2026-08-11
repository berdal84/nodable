#include "Action_Manager.h"
#include "core/Event.h"
#include "core/Event_Manager.h"
#include "tools/core/Asserts.h"
#include "tools/core/Log.h"

tools::Action_Manager* g_action_manager = nullptr;

tools::Action_Manager* tools::action_manager_init()
{
    VERIFY(g_action_manager == nullptr, "Cannot be called twice");
    g_action_manager = new Action_Manager();
    return g_action_manager;
}

tools::Action_Manager* tools::action_manager_get()
{
    VERIFY(g_action_manager != nullptr, "event manager can't be found. Did you call set_name ?");
    return g_action_manager;
}

void tools::action_manager_deinit(Action_Manager* action_manager)
{
    ASSERT(action_manager == g_action_manager);
    ASSERT(g_action_manager != nullptr);
    delete g_action_manager;
    g_action_manager = nullptr;
}

const tools::Action* tools::action_manager_get_action_with_event_type(const Action_Manager* action_manager, Event_Type id)
{
    auto found = action_manager->actions_index_by_event_type.find(id);
    if ( found == action_manager->actions_index_by_event_type.end() )
    {
        String_128 str;
        str.append_fmt("Unable to find an action bound to EventId %i\n", id);
        VERIFY(false, str.c_str() );
    }
    size_t pos = found->second;
    return &action_manager->actions[pos];
}

void tools::action_manager_add_action(Action_Manager* action_manager, const Action& action )// Add a new action (can be triggered via shortcut)
{
    size_t pos = action_manager->actions.size();
    action_manager->actions.push_back( action );
    action_manager->actions_index_by_event_type.insert(std::pair{action.event.type, pos});
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Action_Manager", "Action '%s' bound to the event.type %i\n", action.label.c_str(), action.event.type);
}

void tools::action_manager_add_action(Action_Manager* action_manager, const char* label, Event_Type event_type, Shortcut shortcut)
{
    action_manager_add_action(action_manager, {{event_type}, label, shortcut});
}

void tools::action_manager_add_action(Action_Manager* action_manager, const char* label, Event event, Shortcut shortcut)
{
    ASSERT(event.type <= Event_Type_COUNT)
    action_manager_add_action(action_manager, {event, label, shortcut});
}

void tools::action_manager_add_action(Action_Manager* action_manager, const char* label, Event_Data__User event_data, Shortcut shortcut, u64_t flags )
{
    Event event{ Event_Type_USER};
    event.user = event_data;
    ASSERT(event_data.code < 20);

    action_manager_add_action(action_manager, {event, label, shortcut, flags});
}