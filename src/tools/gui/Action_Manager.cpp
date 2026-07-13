#include "Action_Manager.h"

#include <SDL_keyboard.h>
#include <future>
#include <thread>

#include "tools/core/Task_Manager.h"
#include "tools/core/Asserts.h"
#include "tools/core/Log.h"

using namespace tools;

Action_Manager* g_action_manager = nullptr;

Action_Manager* tools::init_action_manager()
{
    VERIFY(g_action_manager == nullptr, "Cannot be called twice");
    g_action_manager = new Action_Manager();
    return g_action_manager;
}

Action_Manager* tools::get_action_manager()
{
    VERIFY(g_action_manager != nullptr, "event manager can't be found. Did you call set_name ?");
    return g_action_manager;
}

void tools::shutdown_action_manager(Action_Manager* _action_manager)
{
    ASSERT(_action_manager == g_action_manager);
    ASSERT(g_action_manager != nullptr);
    delete g_action_manager;
    g_action_manager = nullptr;
}

Action_Manager::~Action_Manager()
{
    for(auto* action : m_actions )
        delete action;
}

const IAction* Action_Manager::get_action_with_id(Event_ID id) const
{
    auto found = m_actions_by_id.find(id);
    if ( found == m_actions_by_id.end() )
    {
        String_128 str;
        string_append_fmt(&str, "Unable to find an action bound to EventId %i\n", id);
        VERIFY(false, str.c_str() );
    }
    return found->second;
}

const std::vector<IAction*>& Action_Manager::get_actions() const
{
    return m_actions;
}

void Action_Manager::add_action( IAction* _action )// Add a new action (can be triggered via shortcut)
{
    m_actions.push_back( _action );
    m_actions_by_id.insert(std::pair{_action->event_id, _action});
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Action_Manager", "Action '%s' bound to the event_id %i\n", _action->label.c_str(), _action->event_id);
}

std::string Shortcut::to_string() const
{
    std::string result;

    if (mod & KMOD_CTRL) result += "Ctrl + ";
    if (mod & KMOD_ALT)  result += "Alt + ";
    if (key)             result += SDL_GetKeyName(key);
    if (!description.empty()) result += description;

    return result;
}
