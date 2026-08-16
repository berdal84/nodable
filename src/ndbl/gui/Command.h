#pragma once

#include "ImGuiColorTextEdit/TextEditor.h"
#include "core/Node.h"
#include "core/Node_Slot.h"
#include "gui/View.h"
#include <vector>

//
// MACROS to help declare new commands
//
#define COMMAND( command_name, Command_Data_Type ) \
ndbl::Command command_##command_name(Command_Data_Type)

#define COMMAND_RELEASE( command_name ) \
void command_group_release(ndbl::Command*)

#define COMMAND_DO( command_name ) \
void command_##command_name##_do(ndbl::Command* command)

#define COMMAND_UNDO( command_name ) \
void command_##command_name##_undo(ndbl::Command* command)

#define COMMAND_REDO( command_name ) \
void command_##command_name##_redo(ndbl::Command* command)

#define COMMAND_DECLARE_WITH_ARGS( command_name, Command_Data_Type ) \
COMMAND( command_name, Command_Data_Type ); \
COMMAND_RELEASE( command_name ); \
COMMAND_DO( command_name ); \
COMMAND_UNDO( command_name ); \
COMMAND_REDO( command_name );

#define COMMAND_DECLARE( command_name ) \
Command command_##command_name(); \
COMMAND_RELEASE( command_name ); \
COMMAND_DO( command_name ); \
COMMAND_UNDO( command_name ); \
COMMAND_REDO( command_name );

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
        const std::string* old_content;
        const std::string* new_content;
    };

    struct Command_Data__Text_Undo_Record
    {
        TextEditor::UndoRecord* undo_record;
        TextEditor*             text_editor;
    };

    struct Command_Data__Selection_Change
    {
        View_Selection* old_selection;
        View_Selection* new_selection;
    };

    typedef int Command_Flags;
    enum Command_Flags_ : int
    {
        Command_Flags_NONE              = 0,
        Command_Flags_TRANSACTION_BEGIN = 1 << 0,
        Command_Flags_TRANSACTION_END   = 1 << 1
    };

    struct Command
    {
        typedef void(Contextual_Procedure_Type)(Command*);
        typedef void(Procedure_Type)(Command*);

        Command_Type    type;
        Command_Flags   flags;
        const char*     description;
        
        union {
            Command_Data                    data;
            Command_Data__Link              link;
            Command_Data__Text_Replace      text_replace;
            Command_Data__Text_Undo_Record  text_undo_record;
            Command_Data__Selection_Change  selection_change;
        };

        Contextual_Procedure_Type*  proc_do      = nullptr;
        Contextual_Procedure_Type*  proc_redo    = nullptr;
        Contextual_Procedure_Type*  proc_undo    = nullptr;
        Procedure_Type*             proc_release = nullptr;
    };

    void        command_release(Command*);
    void        command_do(Command*);
    void        command_redo(Command*);
    void        command_undo(Command*);

    COMMAND_DECLARE_WITH_ARGS(connect           , Command_Data__Link)
    COMMAND_DECLARE_WITH_ARGS(disconnect        , Command_Data__Link)
    COMMAND_DECLARE_WITH_ARGS(replace_text      , Command_Data__Text_Replace)
    COMMAND_DECLARE_WITH_ARGS(text_undo_record  , Command_Data__Text_Undo_Record)
    COMMAND_DECLARE_WITH_ARGS(new_node          , Command_Data)
    COMMAND_DECLARE_WITH_ARGS(selection_change  , const View_Selection* new_selection)
}


