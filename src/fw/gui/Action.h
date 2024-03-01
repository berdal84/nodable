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
    class Action
    {
    public:
        Action(
                const char*     label,
                EventID         event_t,
                const Shortcut& shortcut = {})
            : label(label)
            , event_id(event_t)
            , shortcut(shortcut)
        {}
        std::string        label;
        EventID            event_id;
        Shortcut           shortcut;
        virtual void*      data() const { return nullptr; } // Pointer to custom data
    };

    /** Generic action with a custom payload */
    template<typename PayloadT>
    class TAction : public Action {
    public:
        static_assert( !std::is_same_v<void, PayloadT> );
        static_assert( !std::is_same_v<nullptr_t , PayloadT> );

        using payload_t = PayloadT;

        TAction(
                const char*  label,
                EventID      event_t,
                Shortcut     shortcut = {},
                PayloadT     payload = {}
                )
            : Action(label, event_t, shortcut)
            , payload(payload)
        {}
        [[nodiscard]] void* data() const override { return (void*)const_cast<PayloadT*>( &payload ); }
        PayloadT payload; // Custom data to attach
    };
}