#include "Action.h"
#include "tools/core/EventManager.h"
#include "tools/core/assertions.h"

using namespace tools;

void IAction::trigger() const
{
    EventManager* event_manager = get_event_manager();
    ASSERT(event_manager != nullptr);
    event_manager->dispatch( make_event() );
}

IEvent* IAction::make_event() const
{
    return new IEvent(event_id);
}