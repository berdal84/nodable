#pragma once
#include "tools/core/types.h"
#include <string>

namespace tools
{
    typedef  u16_t EventID;
    /**
     * Enumerate event types
     * Can be extended starting at EventID_USER_DEFINED
     */
    enum EventID_ : u16_t
    {
        // Declare common event types

        EventID_NULL = 0,

        EventID_FILE_SAVE,
        EventID_FILE_SAVE_AS,
        EventID_FILE_NEW,
        EventID_FILE_CLOSE,
        EventID_FILE_BROWSE,
        EventID_UNDO,
        EventID_REDO,
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

    struct null_data_t {};

    /** Template to extend IEvent with a specific payload */
    template<EventID id_value, typename DataT = null_data_t>
    class Event : public IEvent
    {
    public:
        constexpr static EventID id = id_value;
        using data_t = DataT; // type required to construct this Event

        DataT data;

        explicit Event(DataT _data = {})
            : IEvent(id_value)
            , data( _data )
        {}
    };

    // Below, few basic events (not requiring any payload)

    using Event_NULL            = Event<EventID_NULL>;
    using Event_FileSave        = Event<EventID_FILE_SAVE>;
    using Event_FileSaveAs      = Event<EventID_FILE_SAVE_AS>;
    using Event_FileClose       = Event<EventID_FILE_CLOSE>;
    using Event_FileBrowse      = Event<EventID_FILE_BROWSE>;
    using Event_FileNew         = Event<EventID_FILE_NEW>;
    using Event_Exit            = Event<EventID_REQUEST_EXIT>;
    using Event_Undo            = Event<EventID_UNDO>;
    using Event_Redo            = Event<EventID_REDO>;

    // Here, an event requiring the following payload

    struct EventPayload_ShowWindow
    {
        std::string window_id;        // String identifying a given  window (user defined)
        bool        visible   = true; // Window visibility (desired state)
    };
    using Event_ShowWindow = Event<EventID_REQUEST_SHOW_WINDOW, EventPayload_ShowWindow>;
}