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
    class BaseAction
    {
    public:
        BaseAction(
                EventID         event_id,
                const char*     label,
                const Shortcut& shortcut = {})
            : label(label)
            , event_id(event_id)
            , shortcut(shortcut)
        {}
        std::string        label;
        EventID            event_id;
        Shortcut           shortcut;
        virtual BaseEvent* make_event() const { return new BaseEvent(event_id); }
    };

    /** Generic Action to trigger a given EventT */
    template<EventID _event_id>
    class SimpleAction : public BaseAction
    {
    public:
        explicit SimpleAction( const char*  label, Shortcut shortcut = {} )
            : BaseAction(event_id, label, shortcut)
        {}
    };

    /** Generic Action able to make a given EventT from an ActionConfigT */
    template<typename EventT>
    class CustomAction : public BaseAction
    {
    public:
        static_assert( !std::is_base_of_v<EventT, BaseEvent> );
        using event_t       = EventT;
        using event_data_t = typename EventT::data_t;
        CustomAction(
                const char*   label,
                Shortcut      shortcut = {},
                event_data_t event_initial_state = {}
                )
            : BaseAction(EventT::id, label, shortcut)
            , event_initial_state( event_initial_state )
        {}
        EventT*  make_event() const override { return new EventT( event_initial_state ); }
        event_data_t event_initial_state; // Custom data to attach
    };

    using Action_FileSave        = SimpleAction<EventID_REQUEST_FILE_SAVE>;
    using Action_FileSaveAs      = SimpleAction<EventID_REQUEST_FILE_SAVE_AS>;
    using Action_FileClose       = SimpleAction<EventID_REQUEST_FILE_CLOSE>;
    using Action_FileBrowse      = SimpleAction<EventID_REQUEST_FILE_BROWSE>;
    using Action_FileNew         = SimpleAction<EventID_REQUEST_FILE_NEW>;
    using Action_Exit            = SimpleAction<EventID_REQUEST_EXIT>;
    using Action_Undo            = SimpleAction<EventID_REQUEST_UNDO>;
    using Action_Redo            = SimpleAction<EventID_REQUEST_REDO>;
    using Action_ShowWindow      = CustomAction<Event_ShowWindow>;
}