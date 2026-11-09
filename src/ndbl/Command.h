#pragma once

#include <vector>

#include "Node_Slot.h"
#include "Node.h"
#include "texteditor/Text_Editor.h"
#include "View.h"

//
// MACROS to help declare new commands
//
#define NDBL_COMMAND( NAME, TYPE ) \
ndbl::Command new_command_##NAME(TYPE)

#define NDBL_COMMAND_RELEASE( NAME ) \
void command_##NAME##_release(ndbl::Command*)

#define NDBL_COMMAND_DO( NAME ) \
void command_##NAME##_do(ndbl::Command* command)

#define NDBL_COMMAND_UNDO( NAME ) \
void command_##NAME##_undo(ndbl::Command* command)

#define NDBL_COMMAND_REDO( NAME ) \
void command_##NAME##_redo(ndbl::Command* command)

#define NDBL_COMMAND_DECLARE_WITH_ARGS( NAME, TYPE ) \
NDBL_COMMAND( NAME, TYPE ); \
NDBL_COMMAND_RELEASE( NAME ); \
NDBL_COMMAND_DO( NAME ); \
NDBL_COMMAND_UNDO( NAME ); \
NDBL_COMMAND_REDO( NAME );

#define NDBL_COMMAND_DECLARE( NAME ) \
ndbl::Command new_command_##NAME(); \
NDBL_COMMAND_RELEASE( NAME ); \
NDBL_COMMAND_DO( NAME ); \
NDBL_COMMAND_UNDO( NAME ); \
NDBL_COMMAND_REDO( NAME );

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
    };

    struct Command_Data
    {
        void*        data1;
        void*        data2;
    };

    struct Command_Data__Link
    {
        Node_Slot*      tail;
        Node_Slot*      head;
    };

    struct Command_Data__Text_Replace
    {
        const bdc::String old_content;
        const bdc::String new_content;
    };

    struct Command_Data__Text_Undo_Record
    {
        Text_Editor::UndoRecord undo_record;
        Text_Editor*            text_editor;
    };

    struct Command_Data__Selection_Change
    {
        Array<View> old_selection;
        Array<View> new_selection;
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

    struct Command
    {
        typedef void(Contextual_Procedure_Type)(Command*);
        typedef void(Procedure_Type)(Command*);

        Command_Type    type           = Command_Type_NULL;
        Command_Flags   flags          = Command_Flags_NONE;
        bdc::String     description    = "";
        
        union {
            Command_Data                    data;
            Command_Data__Link              link;
            Command_Data__Text_Replace*     text_replace;
            Command_Data__Text_Undo_Record* text_undo_record;
            Command_Data__Selection_Change* selection_change;
        };

        Contextual_Procedure_Type*  proc_do      = nullptr;
        Contextual_Procedure_Type*  proc_redo    = nullptr;
        Contextual_Procedure_Type*  proc_undo    = nullptr;
        Procedure_Type*             proc_release = nullptr;
    };



    NDBL_COMMAND_DECLARE_WITH_ARGS(connect           , const Command_Data__Link&)
    NDBL_COMMAND_DECLARE_WITH_ARGS(disconnect        , const Command_Data__Link&)
    NDBL_COMMAND_DECLARE_WITH_ARGS(replace_text      , const Command_Data__Text_Replace&)
    NDBL_COMMAND_DECLARE_WITH_ARGS(text_undo_record  , const Command_Data__Text_Undo_Record&)
    NDBL_COMMAND_DECLARE_WITH_ARGS(new_node          , const Command_Data&)
    NDBL_COMMAND_DECLARE_WITH_ARGS(selection_change  , const Array<View>& new_selection)
}


