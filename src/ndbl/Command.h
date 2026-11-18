#pragma once

#include <vector>

#include "Node_Slot.h"
#include "Node.h"
#include "texteditor/Text_Editor.h"
#include "View.h"

//
// MACROS to help declare new commands
//
#define NDBL_COMMAND( NAME, ... ) \
ndbl::Command make_cmd_##NAME(__VA_ARGS__)

#define NDBL_COMMAND_RELEASE( NAME, ... ) \
void command_##NAME##_release(ndbl::Command* __VA_ARGS__)

#define NDBL_COMMAND_DO( NAME, ... ) \
void command_##NAME##_do(ndbl::Command* __VA_ARGS__)

#define NDBL_COMMAND_UNDO( NAME, ... ) \
void command_##NAME##_undo(ndbl::Command* __VA_ARGS__)

#define NDBL_COMMAND_REDO( NAME, ... ) \
void command_##NAME##_redo(ndbl::Command* __VA_ARGS__)

#define NDBL_COMMAND_DECLARE_UNDOABLE( NAME, ... ) \
NDBL_COMMAND( NAME __VA_OPT__(,) __VA_ARGS__ ); \
NDBL_COMMAND_RELEASE( NAME ); \
NDBL_COMMAND_DO( NAME ); \
NDBL_COMMAND_UNDO( NAME ); \
NDBL_COMMAND_REDO( NAME );

#define NDBL_COMMAND_DECLARE_DOABLE( NAME, ... ) \
NDBL_COMMAND( NAME __VA_OPT__(,) __VA_ARGS__ ); \
NDBL_COMMAND_RELEASE( NAME ); \
NDBL_COMMAND_DO( NAME );

namespace ndbl
{
    // forward declarations
    struct Context;
    struct Command;
    struct Node_Slot;
    struct File;

    struct Context // any Command gets a Context* in do/undo/redo procedures
    {
        File* current_file;
    };

    typedef int Command_Type;
    enum Command_Type_
    {
        Command_Type_NULL  = 0,
        Command_Type_CONNECT,
        Command_Type_DISCONNECT,
        Command_Type_TEXT_REPLACE,
        Command_Type_TEXT_UNDO_RECORD,
        Command_Type_SELECTION_CHANGE,
        Command_Type_SET_NODE_VIEW_POSITION,
    };

    struct Cmd_Data
    {
        void*        data1;
        void*        data2;
    };

    struct Cmd_Link
    {
        Node_Slot*      tail;
        Node_Slot*      head;
    };

    struct Cmd_Text_Replace
    {
        const bdc::String old_content;
        const bdc::String new_content;
    };

    struct Cmd_Text_Undo_Record
    {
        Text_Editor::UndoRecord undo_record;
        Text_Editor*            text_editor;
    };

    struct Cmd_Selection_Change
    {
        Array<View> old_selection;
        Array<View> new_selection;
    };

    inline Cmd_Selection_Change* cmd_selection_change_copy(Cmd_Selection_Change* out, const Cmd_Selection_Change* in)
    {
        out->old_selection = array_copy(in->old_selection);
        out->new_selection = array_copy(in->new_selection);
        return out;
    };

    struct Cmd_New_Node
    {
        Node_Init_Params node_init_params;
    };

    struct Set_Node_View_Position // could be Set_Spatial_Node_Position
    {
        Node_ID node_id; // could be spatial_node_id
        float   x, y;
        Space   space;
    };

    typedef int Command_Flags;
    enum Command_Flags_ : int
    {
        Command_Flags_NONE              = 0,
        Command_Flags_TRANSACTION_BEGIN = 1 << 0,
        Command_Flags_TRANSACTION_END   = 1 << 1
    };

    void        command_release(Command*);
    void        command_do(Command*);
    void        command_redo(Command*);
    void        command_undo(Command*);

    struct Cmd_Push_Params
    {
        bool skip_command_do_at_push;
    };

    struct Command
    {
        typedef void(Contextual_Procedure_Type)(Command*);
        typedef void(Procedure_Type)(Command*);

        Command_Type    type           = Command_Type_NULL;
        Command_Flags   flags          = Command_Flags_NONE;
        bdc::String     description    = "";
        
        union {
            Cmd_Data                data;
            Cmd_New_Node            new_node;
            Cmd_Link                link;
            Set_Node_View_Position  set_node_view_position;
            Cmd_Text_Replace*       text_replace;
            Cmd_Text_Undo_Record*   text_undo_record;
            Cmd_Selection_Change*   selection_change;
        } data;

        Contextual_Procedure_Type*  proc_do      = nullptr;
        Contextual_Procedure_Type*  proc_redo    = nullptr;
        Contextual_Procedure_Type*  proc_undo    = nullptr;
        Procedure_Type*             proc_release = nullptr;
    };

    NDBL_COMMAND_DECLARE_UNDOABLE(connect           , Cmd_Link)
    NDBL_COMMAND_DECLARE_UNDOABLE(disconnect        , Cmd_Link)
    NDBL_COMMAND_DECLARE_UNDOABLE(replace_text      , const Cmd_Text_Replace&)
    NDBL_COMMAND_DECLARE_UNDOABLE(text_undo_record  , const Cmd_Text_Undo_Record&)
    NDBL_COMMAND_DECLARE_UNDOABLE(new_node          , const Node_Init_Params&)
    NDBL_COMMAND_DECLARE_UNDOABLE(selection_change  , const Cmd_Selection_Change&)
    NDBL_COMMAND_DECLARE_DOABLE(set_node_view_position, const Set_Node_View_Position&)
}


