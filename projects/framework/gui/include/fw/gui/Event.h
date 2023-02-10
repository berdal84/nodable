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

    struct BindedEvent
    {
        std::string label;
        uint16_t event_t;
        Shortcut shortcut;
        uint16_t condition;
    };

    class EventManager
    {
    public:
        EventManager();
        ~EventManager();
        void               push_event(Event& _event);
        size_t             poll_event(Event& _event);
        void               push_event(EventType _type);
        void                            bind(const BindedEvent& binded_cmd);
        const BindedEvent&              get_binded(uint16_t type);
        const std::vector<BindedEvent>& get_binded_events() const;
        static EventManager&            get_instance();

    private:
        static EventManager*            s_instance;
        std::queue<Event>               m_events;
        std::vector<BindedEvent>        m_binded_events;
        std::map<uint16_t, BindedEvent> m_binded_events_by_type;
    };
}