#pragma once
#include "Types.h"
#include <string>

namespace tools
{
    typedef  u16_t Event_ID;
    /**
     * Enumerate event types
     * Can be extended starting at Event_ID_USER_DEFINED
     */
    enum Event_ID_ : u16_t
    {
        // Declare common event types

        Event_ID_NULL = 0,

        Event_ID_FILE_SAVE,
        Event_ID_FILE_SAVE_AS,
        Event_ID_FILE_NEW,
        Event_ID_FILE_CLOSE,
        Event_ID_FILE_BROWSE,
        Event_ID_UNDO,
        Event_ID_REDO,
        Event_ID_REQUEST_EXIT,
        Event_ID_REQUEST_SHOW_WINDOW,

        Event_ID_FILE_OPENED,

        Event_ID_USER_DEFINED = 0xff,
    };

    /** Basic event, can be extended via CustomEvent */
    class IEvent
    {
    public:
        const Event_ID id;
        constexpr explicit IEvent(Event_ID id): id(id) {}
        virtual ~IEvent() = default;
    };

    struct null_data_t {};

    /** Template to extend IEvent with a specific payload */
    template<Event_ID id_value, typename DataT = null_data_t>
    class Event : public IEvent
    {
    public:
        constexpr static Event_ID id = id_value;
        using data_t = DataT; // type required to construct this Event

        DataT data;

        explicit Event(DataT _data = {})
            : IEvent(id_value)
            , data( _data )
        {}
    };

    // Below, few basic events (not requiring any payload)

    using Event_NULL            = Event<Event_ID_NULL>;
    using Event_FileSave        = Event<Event_ID_FILE_SAVE>;
    using Event_FileSaveAs      = Event<Event_ID_FILE_SAVE_AS>;
    using Event_FileClose       = Event<Event_ID_FILE_CLOSE>;
    using Event_FileBrowse      = Event<Event_ID_FILE_BROWSE>;
    using Event_FileNew         = Event<Event_ID_FILE_NEW>;
    using Event_Exit            = Event<Event_ID_REQUEST_EXIT>;
    using Event_Undo            = Event<Event_ID_UNDO>;
    using Event_Redo            = Event<Event_ID_REDO>;

    // Here, an event requiring the following payload

    struct Event_Payload__Show_Window
    {
        std::string window_id;        // String identifying a given  window (user defined)
        bool        visible   = true; // Window visibility (desired state)
    };
    using Event_ShowWindow = Event<Event_ID_REQUEST_SHOW_WINDOW, Event_Payload__Show_Window>;
}