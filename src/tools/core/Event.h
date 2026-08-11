#pragma once
#include "Types.h"

namespace tools
{
    typedef u16_t Event_Type;
    enum Event_Type_ : u16_t
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

        // extended events (with data)
        Event_Type_WINDOW,
        Event_Type_USER,

        Event_Type_COUNT
    };

    struct Event_Data__Window
    {
        const char* window_id;        // ImGui identifier
        bool        visible   = true; // desired state
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
            Event_Data__Window  window;
            Event_Data__User    user;
        };

        operator bool ()
        { return type != Event_Type_NULL; }
    };
}