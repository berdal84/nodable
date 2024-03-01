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
        EventID_REQUEST_SHOW_WINDOW,

        EventID_FILE_OPENED,

        EventID_USER_DEFINED = 0xff,
    };

    /** Basic event, can be extended via CustomEvent */
    class IEvent
    {
    public:
        const EventID id;
        constexpr explicit IEvent(EventID id): id(id) {}
        virtual ~IEvent() = default;
    };

    template<EventID event_id>
    class BasicEvent : public fw::IEvent
    {
    public:
        using data_t = struct {};
        constexpr static EventID id = static_cast<EventID>(event_id);

        BasicEvent()
            : IEvent(event_id)
        {}
    };

    /** Template to extend IEvent with a specific payload */
    template<EventID event_id, typename DataT>
    class CustomEvent : public fw::IEvent
    {
    public:
        constexpr static EventID id = event_id;
        using data_t = DataT;

        DataT data;

        CustomEvent( DataT _data = {})
            : IEvent(event_id)
            , data( _data )
        {}
    };

    struct EventPayload_ShowWindow
    {
        std::string window_id; // String identifying a given  window (user defined)
        bool        visible;   // Desired state
    };
    using Event_ShowWindow = CustomEvent<EventID_REQUEST_SHOW_WINDOW, EventPayload_ShowWindow>;
}