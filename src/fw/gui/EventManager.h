#pragma once

#include <queue>
#include <string>
#include <future>
#include <map>
#include <utility>
#include <SDL/include/SDL_keycode.h>

#include "core/types.h"
#include "core/reflection/func_type.h"

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
    typedef u16_t EventType;
    enum EventType_ : u16_t
    {
        // Declare common event types

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

    // An action can trigger an event under certain circumstances
    class Action {
    public:
        std::string      label;
        u16_t            event_t;
        Shortcut         shortcut;
        u16_t             condition;
        const fw::func_type* signature; // If action creates a node, this will point to the signature of if.

        Action(
                const char* label,
                u16_t event_t,
                Shortcut shortcut = {},
                u16_t condition = {},
                const fw::func_type* signature = nullptr)
        : label(label)
        , event_t(event_t)
        , shortcut(std::move(shortcut))
        , condition(condition)
        , signature(signature)
        {}
    };

    class EventManager
    {
    public:
        EventManager();
        EventManager(const EventManager&) = delete;
        ~EventManager();

        void               push_event(EventType _type);
        void               push_event_delayed(EventType, u64_t); // Push an event with a delay in millisecond
        void               push_event(Event& _event);
        size_t             poll_event(Event& _event);
        const std::vector<Action>& get_actions() const;
        void                       add_action(Action);
        const Action&              get_action_by_type(u16_t type);

        static EventManager&       get_instance();

    private:
        static EventManager*       s_instance;
        std::queue<Event>          m_events;
        std::vector<Action>        m_actions;
        std::map<u16_t, Action>    m_actions_by_event_type;
    };
}