#pragma once
#include "core/types.h"

namespace fw
{
    typedef  u16_t EventID;
    /**
     * Enumerate event types
     * Can be extended starting at EventID_USER_DEFINED
     */
    enum EventID_ : u16_t
    {
        // Declare common event types

        EventID_NONE = 0,

        EventID_REQUEST_FILE_SAVE,
        EventID_REQUEST_FILE_SAVE_AS,
        EventID_REQUEST_FILE_NEW,
        EventID_REQUEST_FILE_CLOSE,
        EventID_REQUEST_FILE_BROWSE,
        EventID_REQUEST_UNDO,
        EventID_REQUEST_REDO,
        EventID_REQUEST_EXIT,
        EventID_REQUEST_SHOW_SLASHSCREEN,

        EventID_FILE_OPENED,

        EventID_USER_DEFINED = 0xff,
    };


    /** Basic event, can be extended via TEvent */
    class Event
    {
    public:
        const EventID id;
        constexpr explicit Event(EventID id): id(id) {}
        [[nodiscard]] virtual void* data() const { return nullptr; }
    };

    /** Template to extend Event with a specific payload */
    template<EventID event_id, typename PayloadT>
    class TEvent : public fw::Event
    {
    public:
        constexpr static EventID id = event_id;
        using payload_t = PayloadT;
        PayloadT payload;
        template<typename ...Args>
        explicit TEvent(Args... args): Event(event_id), payload(args...){}
        [[nodiscard]] void* data() const override { return const_cast<void*>( static_cast<const void*>( &payload ) ); }
    };
}