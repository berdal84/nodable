#pragma once
#include "Shortcut.h"
#include <map>
#include <vector>

namespace ndbl
{
    using Condition = int;
    enum Condition_ : int
    {
        Condition_DISABLE                          = 0,
        Condition_ENABLE_IF_HAS_SELECTION          = 1 << 0,
        Condition_ENABLE_IF_HAS_NO_SELECTION       = 1 << 1,
        Condition_ENABLE_IF_HAS_GRAPH              = 1 << 3,
        Condition_ENABLE                           = Condition_ENABLE_IF_HAS_SELECTION
                                                   | Condition_ENABLE_IF_HAS_NO_SELECTION
                                                   | Condition_ENABLE_IF_HAS_GRAPH,
        Condition_HIGHLIGHTED_IN_GRAPH_EDITOR      = 1 << 4,
        Condition_HIGHLIGHTED_IN_TEXT_EDITOR       = 1 << 5,
        Condition_HIGHLIGHTED                      = Condition_HIGHLIGHTED_IN_GRAPH_EDITOR
                                                   | Condition_HIGHLIGHTED_IN_TEXT_EDITOR,
    };

    struct BindedEvent {
        std::string label;
        EventType event_t;
        Shortcut shortcut;
        Condition condition = Condition_ENABLE;
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