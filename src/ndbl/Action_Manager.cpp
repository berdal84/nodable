#include "Action_Manager.h"

#include "Asserts.h"
#include "Event.h"
#include "Log.h"

#define ASSERT_ACTION_MANAGER_IS_INITIALIZED() VERIFY(g_action_manager != nullptr, "event manager can't be found. Did you call action_manager_init ?")

// private
namespace ndbl
{
    static Action_Manager* g_action_manager = nullptr;

    void* _action_manager_set(Action_Manager*);
}

namespace ndbl
{
    using namespace bdc;

    Action_Manager* action_manager_init()
    {
        VERIFY(g_action_manager == nullptr, "Only works once, you can't call init when a current action manager is set.");

        g_action_manager = bdc::memory_new<Action_Manager>();
        hashmap_init(g_action_manager->actions_by_event_type);
        
        return g_action_manager;
    }

    void action_manager_set_current(Action_Manager* action_manager)
    {
        VERIFY(action_manager != nullptr, "action_manager cant' be null!");
        g_action_manager = action_manager;
    }

    Action_Manager* action_manager()
    {
        ASSERT_ACTION_MANAGER_IS_INITIALIZED();
        return g_action_manager;
    }

    void action_manager_shutdown()
    {
        ASSERT_ACTION_MANAGER_IS_INITIALIZED();
        hashmap_release(g_action_manager->actions_by_event_type);
        bdc::memory_delete( g_action_manager );
        g_action_manager = nullptr;
    }

    const Action* action_manager_get_action_with_event_type(Event_Type type)
    {
        ASSERT_ACTION_MANAGER_IS_INITIALIZED();
        auto found = hashmap_find(g_action_manager->actions_by_event_type, type);
        if ( !found.ok )
        {
            String str = string_printf("Unable to find an action bound to EventId %i\n", type);
            VERIFY(false, str.c_str() );
        }
        return &g_action_manager->actions[*found.value];
    }

    void action_manager_register_action(const Action& action )// Add a new action (can be triggered via shortcut)
    {
        ASSERT_ACTION_MANAGER_IS_INITIALIZED();
        
        size_t pos = g_action_manager->actions.size;
        
        if( !HAS_FLAGS(action.flags, Action_Flags_DISABLE_INDEXING ) )
        {
            auto found = hashmap_find_index(g_action_manager->actions_by_event_type, action.event.type);
            ASSERT(!found.ok && "We should only have one action per event type in the index."); 
            hashmap_add(g_action_manager->actions_by_event_type, action.event.type, pos);
        }

        array_append(g_action_manager->actions, action );

        NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Action_Manager", "Action '%s' bound to the event.type %i\n", action.label.c_str(), action.event.type);
    }
} // namespace ndbl