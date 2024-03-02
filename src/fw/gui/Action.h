#pragma once
#include "Event.h"
#include "core/types.h"
#include <SDL/include/SDL_keycode.h>

namespace fw
{
    /** Data describing a shortcut (ex: "Reset": Ctrl + Alt + R) */
    struct Shortcut
    {
        SDL_Keycode key         = SDLK_UNKNOWN;    // a key to be pressed
        SDL_Keymod  mod         = KMOD_NONE;       // modifiers (alt, ctrl, etc.)
        std::string description;
        std::string to_string() const;
    };

    /** Basic action defining which event has to be triggered when a given shortcut is detected */
    class IAction
    {
    public:
        IAction(
            EventID         event_id,
            const char*     label,
            const Shortcut& shortcut = {},
            u64_t           userdata = {}
        )
        : label(label)
        , event_id(event_id)
        , shortcut(shortcut)
        , userdata(userdata)
        {}
        std::string label;
        EventID     event_id;
        Shortcut    shortcut;
        u64_t       userdata;
        virtual IEvent* make_event() const { return new IEvent(event_id); }
    };

    /** Generic Action to trigger a given EventT */
    template<EventID _event_id>
    class Action : public IAction
    {
    public:
        using event_t = fw::Event<_event_id>;
        using event_data_t = typename event_t::data_t;
        explicit Action(
            const char*  label,
            Shortcut     shortcut = {},
            u64_t        userdata = {}
        )
            : IAction(event_id, label, shortcut, userdata)
        {}
    };

    /** Generic Action able to make a given EventT from an ActionConfigT */
    template<typename EventT>
    class CustomAction : public IAction
    {
    public:
        static_assert( !std::is_base_of_v<EventT, IEvent> );
        using event_t      = EventT;
        using event_data_t = typename EventT::data_t;
        CustomAction(
                const char*   label,
                Shortcut      shortcut,
                event_data_t  event_initial_state = {},
                u64_t         userdata = {}
        )
            : IAction(EventT::id, label, shortcut, userdata)
            , event_initial_state( event_initial_state )
        {}
        EventT*  make_event() const override { return new EventT( event_initial_state ); }
        event_data_t event_initial_state; // Custom data to attach
    };

    using Action_FileSave        = Action<EventID_REQUEST_FILE_SAVE>;
    using Action_FileSaveAs      = Action<EventID_REQUEST_FILE_SAVE_AS>;
    using Action_FileClose       = Action<EventID_REQUEST_FILE_CLOSE>;
    using Action_FileBrowse      = Action<EventID_REQUEST_FILE_BROWSE>;
    using Action_FileNew         = Action<EventID_REQUEST_FILE_NEW>;
    using Action_Exit            = Action<EventID_REQUEST_EXIT>;
    using Action_Undo            = Action<EventID_REQUEST_UNDO>;
    using Action_Redo            = Action<EventID_REQUEST_REDO>;
    using Action_ShowWindow      = CustomAction<Event_ShowWindow>;
}