#pragma once

namespace ndbl
{
    enum Event_Type: u32_t
    {
        Event_Type_NULL                                 = 0,

        //....................................................
        // requests (intent to..)
        //....................................................  

        Event_Type_EXIT_PROGRAM_REQUEST                 =   1,
        Event_Type_TOGGLE_HELP_REQUEST                  =   2,
        Event_Type_TOGGLE_FULLSCREEN_REQUEST            =   3,
        Event_Type_UNDO_REQUEST                         =   4,
        Event_Type_REDO_REQUEST                         =   5,   

        Event_Type_FILE_SAVE_REQUEST                    = 201,
        Event_Type_FILE_SAVE_AS_REQUEST                 = 202,
        Event_Type_FILE_NEW_REQUEST                     = 203,
        Event_Type_FILE_CLOSE_REQUEST                   = 204,
        Event_Type_FILE_BROWSE_REQUEST                  = 205,

        Event_Type_RESET_LAYOUT_REQUEST                 = 300, 
        Event_Type_DELETE_ALL_LINKS_REQUEST             = 301,
        Event_Type_DELETE_LINK_REQUEST                  = 302,
        Event_Type_DELETE_REQUEST                       = 303,
        Event_Type_MOVE_REQUEST                         = 304,
        Event_Type_CREATE_NODE_REQUEST                  = 305,
        Event_Type_FRAME_SELECTION_REQUEST              = 306,
        Event_Type_RESET_GRAPH_VIEW_REQUEST             = 307,
        Event_Type_SELECT_NEXT_REQUEST                  = 308,
        Event_Type_SELECTION_CHANGE_REQUEST             = 309,
        Event_Type_TOGGLE_FOLDING_REQUEST               = 311,
        Event_Type_TOGGLE_ISOLATION_REQUEST             = 312,
       
        //....................................................
        // notification (from the past)
        //....................................................  

        Event_Type_FILE_OPENED                          = 1000,
    };

    struct Event
    {
        Event_Type type;
        u32_t      code;
        void*      data1;
        void*      data2;
    };
}