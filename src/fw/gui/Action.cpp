#include "Action.h"
#include "EventManager.h"

using namespace fw;

void IAction::trigger() const
{
    IEvent* event = make_event();
    EventManager::get_instance().dispatch( event );
}

IEvent* IAction::make_event() const
{
    return new IEvent(event_id);
}