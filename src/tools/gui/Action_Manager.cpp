#include "Action_Manager.h"

#include "core/Event.h"
#include "tools/core/Asserts.h"
#include "tools/core/Log.h"

#define ASSERT_ACTION_MANAGER_IS_INITIALIZED() VERIFY(tools::g_action_manager != nullptr, "event manager can't be found. Did you call action_manager_init ?")

// private
namespace tools
{
    static Action_Manager* g_action_manager = nullptr;

    void* _action_manager_set(Action_Manager*);
}

tools::Action_Manager* tools::action_manager_init()
{
    VERIFY(g_action_manager == nullptr, "Only works once, you can't call init when a current action manager is set.");

    g_action_manager = new Action_Manager();

    return g_action_manager;
}

void tools::action_manager_set_current(Action_Manager* action_manager)
{
    VERIFY(action_manager != nullptr, "action_manager cant' be null!");
    g_action_manager = action_manager;
}

tools::Action_Manager* tools::action_manager()
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    return g_action_manager;
}

void tools::action_manager_shutdown()
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    delete g_action_manager;
    g_action_manager = nullptr;
}

const tools::Action* tools::action_manager_get_action_with_event_type(Event_Type id)
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    auto found = g_action_manager->actions_index_by_event_type.find(id);
    if ( found == g_action_manager->actions_index_by_event_type.end() )
    {
        String_128 str;
        str.append_fmt("Unable to find an action bound to EventId %i\n", id);
        VERIFY(false, str.c_str() );
    }
    size_t pos = found->second;
    return &g_action_manager->actions[pos];
}

void tools::action_manager_add_action(const Action& action )// Add a new action (can be triggered via shortcut)
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    size_t pos = g_action_manager->actions.size();
    g_action_manager->actions.push_back( action );
    g_action_manager->actions_index_by_event_type.insert(std::pair{action.event.type, pos});
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Action_Manager", "Action '%s' bound to the event.type %i\n", action.label.c_str(), action.event.type);
}

void tools::action_manager_add_action(const char* label, Event_Type event_type, Shortcut shortcut)
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    action_manager_add_action({{event_type}, label, shortcut});
}

void tools::action_manager_add_action(const char* label, Event event, Shortcut shortcut)
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    ASSERT(event.type <= Event_Type_COUNT);
    action_manager_add_action({event, label, shortcut});
}

void tools::action_manager_add_action(const char* label, Event_Data__User event_data, Shortcut shortcut, u64_t flags )
{
    ASSERT_ACTION_MANAGER_IS_INITIALIZED();
    Event event = event_from_user_data(event_data);
    Action action(event, label, shortcut, flags);
    action_manager_add_action(action);
}