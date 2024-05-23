#include "Action.h"
#include "EventManager.h"

using namespace tools;

void IAction::trigger() const
{
    EventManager& event_manager = EventManager::get_instance();
    event_manager.dispatch( make_event() );
}

IEvent* IAction::make_event() const
{
    return new IEvent(event_id);
}