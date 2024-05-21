#pragma once

#include <SDL_keycode.h>
#include <string>

#include "fw/core/types.h"
#include "Event.h"

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

    /**
     * The purpose of any IAction is to trigger a given basic event (identified by an EventID)
     * when a key shortcut is pressed.
     */
    class IAction
    {
    public:
        explicit IAction(
            EventID         event_id,
            const char*     label    = "action",
            Shortcut&&      shortcut = {},
            u64_t           userdata = {}
        )
        : label(label)
        , event_id(event_id)
        , shortcut(std::move(shortcut))
        , userdata(userdata)
        {}
        std::string label;
        EventID     event_id;
        Shortcut    shortcut;
        u64_t       userdata;

        void            trigger() const;    // Trigger action, will dispatch an event with default values
        virtual IEvent* make_event() const; // Make a new event with default values
    };

    /**
     * The purpose of an Action is similar to IAction for events requiring some data to be constructed
     */
    template<typename EventT>
    class Action : public IAction
    {
    public:
        static_assert( !std::is_base_of_v<EventT, IEvent> ); // Ensure EventT implements IEvent

        using event_t      = EventT;                  // Type of the event triggered by this action
        using event_data_t = typename EventT::data_t; // Type of the payload for the events triggered by this action

        event_data_t event_data; // Initial data used when making a new event

        Action(
            const char* label,
            Shortcut&&  shortcut
            )
            : IAction(EventT::id, label, std::move(shortcut) )
        {}

        Action(
            const char*   label, // Text to display for this action
            Shortcut&&    shortcut, // Shortcut able to trigger this action
            u64_t         userdata // Custom user data (typically to store flags)
            )
            : IAction(EventT::id, label, std::move(shortcut), userdata)
        {}

        Action(
            const char*   label, // Text to display for this action
            Shortcut&&    shortcut, // Shortcut able to trigger this action
            event_data_t  event_data, // Initial data of a default event
            u64_t         userdata = {} // Custom user data (typically to store flags)
        )
        : IAction(EventT::id, label, std::move(shortcut), userdata)
        , event_data( event_data )
        {}

        EventT*  make_event() const override // Make a new event using default event_data
        { return new EventT( event_data ); }
    };
}