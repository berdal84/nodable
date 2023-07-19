#pragma once

#include <queue>
#include <string>
#include <future>

#include <SDL/include/SDL_keycode.h>
#include "types.h"

namespace fw
{
    struct Shortcut
    {
        SDL_Keycode key         = SDLK_UNKNOWN;    // a key to be pressed
        SDL_Keymod  mod         = KMOD_NONE;       // modifiers (alt, ctrl, etc.)
        std::string description;
        std::string to_string() const;
    };


    // Declare some event types.
    // EventType can be extended starting at EventType_USER_DEFINED
    typedef uint16_t EventType;
    enum EventType_                           // Declare common event types
    {
        EventType_none = 0,
        EventType_save_file_triggered,        // operation on files
        EventType_save_file_as_triggered,
        EventType_new_file_triggered,
        EventType_close_file_triggered,
        EventType_browse_file_triggered,
        EventType_file_opened,
        EventType_undo_triggered,            // history
        EventType_redo_triggered,
        EventType_exit_triggered,            // general
        EventType_show_splashscreen_triggered,

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
        EventManager(const EventManager&) = delete;
        ~EventManager();

        void               push_event(Event& _event);
        size_t             poll_event(Event& _event);
        void               push(EventType _type);
        void               push_async(EventType, u64_t); // Push an event with a delay in millisecond
        const std::vector<BindedEvent>& get_binded_events() const;
        void                            bind(const BindedEvent& binded_cmd);
        const BindedEvent&              get_binded(uint16_t type);

        static EventManager&            get_instance();

    private:
        static EventManager*            s_instance;
        std::queue<Event>               m_events;
        std::vector<BindedEvent>        m_binded_events;
        std::map<uint16_t, BindedEvent> m_binded_events_by_type;
    };
}