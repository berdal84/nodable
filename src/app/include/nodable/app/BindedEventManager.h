#pragma once
#include "Shortcut.h"
#include <map>
#include <vector>

namespace ndbl
{
    enum Condition
    {
        Condition_NEVER = 0,
        Condition_ALWAYS = 1 << 0,
        Condition_HAS_SELELECTION = 1 << 1,
        Condition_HAS_NO_SELECTION = 1 << 2
    };

    struct BindedEvent {
        std::string label;
        EventType event_t;
        Shortcut shortcut;
        Condition condition = Condition_NEVER;
    };

    /**
     * Class to manage all commands
     */
    class BindedEventManager
    {
    public:
        static void bind(const BindedEvent &binded_cmd)
        {
            s_binded_events.push_back(binded_cmd);
            s_binded_events_by_type.insert({binded_cmd.event_t, binded_cmd});
        }
        static std::vector<BindedEvent> s_binded_events;
        static std::map<EventType, BindedEvent> s_binded_events_by_type;
        static const BindedEvent& get_event(EventType type)
        {
            return s_binded_events_by_type.at(type);
        }
    };
}// namespace ndbl