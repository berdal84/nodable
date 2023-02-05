#pragma once

#include <fw/gui/Shortcut.h>
#include <fw/gui/types.h>
#include <queue>

namespace fw
{
    typedef uint16_t EventType;
    enum EventType_
    {
        EventType_none = 0,
        EventType_save_file_triggered,        // operation on files
        EventType_save_file_as_triggered,
        EventType_new_file_triggered,
        EventType_close_file_triggered,
        EventType_browse_file_triggered,
        EventType_undo_triggered,
        EventType_redo_triggered,
        EventType_file_opened,
        EventType_exit_triggered,            // general
        EventType_show_splashscreen_triggered,
        EventType_toggle_isolate_selection,

        EventType_USER_DEFINED = 0xff,
    };

    struct SimpleEvent {
        EventType type;
    };

    union Event
    {
        EventType type;
        SimpleEvent common;
    };

    class EventManager
    {
    public:
        static void   push_event(Event& _event);
        static size_t poll_event(Event& _event);
        static void   push_event(EventType _type);

    private:
        static std::queue<Event> s_events;
    };

    struct BindedEvent {
        std::string label;
        uint16_t event_t;
        Shortcut shortcut;
        uint16_t condition;
    };

    /**
     * Class to manage all commands
     */
    class BindedEventManager
    {
    public:

        static void               bind(const BindedEvent& binded_cmd);
        static const BindedEvent& get_event(uint16_t type);

        static std::vector<BindedEvent>        s_binded_events;
        static std::map<uint16_t, BindedEvent> s_binded_events_by_type;
    };
}