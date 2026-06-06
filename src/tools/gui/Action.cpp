#include "Action.h"
#include "tools/core/Event_Manager.h"
#include "tools/core/Asserts.h"

using namespace tools;

void IAction::trigger() const
{
    Event_Manager* event_manager = get_event_manager();
    ASSERT(event_manager != nullptr);
    event_manager->dispatch( make_event() );
}

IEvent* IAction::make_event() const
{
    return new IEvent(event_id);
}