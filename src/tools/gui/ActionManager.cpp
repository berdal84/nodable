#include "ActionManager.h"

#include <SDL_keyboard.h>
#include <future>
#include <thread>

#include "tools/core/assertions.h"
#include "tools/core/async.h"
#include "tools/core/log.h"
#include "tools/core/reflection/type.h"

using namespace tools;

ActionManager* ActionManager::s_instance = nullptr;

ActionManager::ActionManager()
{
    LOG_VERBOSE("tools::ActionManager", "Constructor ...\n");
    EXPECT(!s_instance, "cannot have two instances at a time");
    s_instance = this;
    LOG_VERBOSE("tools::ActionManager", "Constructor " OK "\n");
}

ActionManager::~ActionManager()
{
    LOG_VERBOSE("tools::ActionManager", "Destructor ...\n");
    s_instance = nullptr;
    for( auto action : m_actions )
    {
        delete action;
    }
    LOG_VERBOSE("tools::ActionManager", "Destructor " OK "\n");
}

ActionManager& ActionManager::get_instance()
{
    EXPECT(s_instance, "No instance found.");
    return *s_instance;
}

const IAction* ActionManager::get_action_with_id(EventID id)
{
    auto found = m_actions_by_id.find(id);
    if ( found == m_actions_by_id.end() )
    {
        string128 str;
        str.append_fmt("Unable to find an action bound to EventId %i\n", id);
        EXPECT(false, str.c_str() );
    }
    return found->second;
}

const std::vector<IAction*>& ActionManager::get_actions() const
{
    return m_actions;
}

void ActionManager::add_action( IAction* _action )// Add a new action (can be triggered via shortcut)
{
    m_actions.push_back( _action );
    m_actions_by_id.insert(std::pair{_action->event_id, _action});
    LOG_MESSAGE("ActionManager", "Action '%s' bound to the event_id %i\n", _action->label.c_str(), _action->event_id);
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
