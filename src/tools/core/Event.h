#pragma once
#include "bdc/Types.hpp"
#include "bdc/String.hpp"
#include <cstring>

namespace tools
{
    typedef u32_t Event_Type;
    enum Event_Type_ : Event_Type
    {
        Event_Type_NULL = 0,

        // basic events (no data)
        Event_Type_FILE_SAVE,
        Event_Type_FILE_SAVE_AS,
        Event_Type_FILE_NEW,
        Event_Type_FILE_CLOSE,
        Event_Type_FILE_BROWSE,
        Event_Type_FILE_OPENED,
        Event_Type_UNDO,
        Event_Type_REDO,
        Event_Type_REQUEST_EXIT,
        Event_Type_TOGGLE_HELP,

        // This slot and above are reserved for user event type/codes
        Event_Type_USER = 512,
    };

    struct Event_Data__Window
    {
        const bdc::String imgui_id;
    };

    typedef u16_t Event_User_Code;

    struct Event_Data__User
    {
        Event_User_Code code;
        void*           data1;
        void*           data2;
    };

    struct Event
    {
        Event_Type type;
        union {
            Event_Data__User    user;
            char                data[sizeof(Event_Data__User)];
        };

        operator bool ()
        { return type != Event_Type_NULL; }
    };

    inline Event event_from_type(Event_Type type)
    {
        Event event{type};
        memset(&event.data, 0, sizeof(event.data));
        return event;
    }

    inline Event event_from_user_data(Event_Data__User user_data)
    {
        Event event = event_from_type(Event_Type_USER);
        event.user = user_data;
        return event;
    }
}