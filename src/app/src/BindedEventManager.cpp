#include <nodable/app/BindedEventManager.h>

using namespace ndbl;

std::vector<BindedEvent>         BindedEventManager::s_binded_events;
std::map<EventType, BindedEvent> BindedEventManager::s_binded_events_by_type;