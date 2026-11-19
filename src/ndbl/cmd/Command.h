#pragma once

#include "ndbl/Asserts.h"
#include "ndbl/Node_Slot.h"
#include "ndbl/Node.h"
#include "ndbl/texteditor/Text_Editor.h"
#include "ndbl/View.h"

namespace ndbl
{
    // forward declarations
    struct Context;
    struct Command;
    struct Node_Slot;
    struct File;

    // General Note:
    // I named structs differently for two categories:
    // - Cmd_Payload_XXX: the ones that can be trivially constructed (and therefore, included in the tagged union as-is).
    // - Cmd_Data_XXX:    the ones that can't, and therefore are used as a pointer in the default tagged union struct.
    // Ideally, I would like to have a single category, but It would require that any of the struct in use are trivial.
    // This means, no custom constructor/destructor/etc.

    struct Context // any Command gets a Context* in do/undo/redo procedures
    {
        File* current_file;
    };

    struct Cmd_Payload__Default
    {
        union {
            void*           data;
            void*           ptr;
            void*           data1;
        };
        void*               data2;
    };

    struct Cmd_Payload__Link
    {
        Node_Slot*          tail;
        Node_Slot*          head;
    };

    struct Cmd_Data__Text_Replace
    {
        const bdc::String   old_content;
        const bdc::String   new_content;
    };

    struct Cmd_Data__Text_Undo_Record
    {
        Text_Editor::UndoRecord undo_record;
        Text_Editor*            text_editor;
    };

    struct Cmd_Payload__New_Node
    {
        Node_Init_Params node_init_params;
    };

    struct Cmd_Payload__Set_Node_View_Position // could be Set_Spatial_Node_Position
    {
        Node_ID             node_id; // could be spatial_node_id
        float               x, y;
        Space               space;
    };

    typedef int Cmd_Flags;
    enum Cmd_Flags_ : int
    {
        Cmd_Flags_NONE              = 0,
        Cmd_Flags_TRANSACTION_BEGIN = 1 << 0,
        Cmd_Flags_TRANSACTION_END   = 1 << 1
    };

    struct Cmd_Push_Params
    {
        bool skip_command_do_at_push;
    };

    struct Command
    {
        typedef void(Cmd_Proc_Type)(Command*);
        
        Cmd_Flags      flags           = Cmd_Flags_NONE;
        bdc::String    description     = "";
        Cmd_Proc_Type* proc_do         = nullptr;
        Cmd_Proc_Type* proc_redo       = nullptr;
        Cmd_Proc_Type* proc_undo       = nullptr;
        Cmd_Proc_Type* proc_release    = nullptr;

        union {
            // Note: we don't need a type enum to discriminate, since each proc_xxx set to a command->proc_xxx pointer knows on which type it is working on.
            Cmd_Payload__Default                    data = {0};
            Cmd_Payload__New_Node                   new_node;
            Cmd_Payload__Link                       link;
            Cmd_Payload__Set_Node_View_Position     set_node_view_position;
        } payload;
    };

    // Command API

    void command_release(Command*);
    void command_do(Command*);
    void command_redo(Command*);
    void command_undo(Command*);

    // Inlined Command API implementation
    inline void command_release(Command* command)
    {
        // Not all commands needs to release memory
        if( command->proc_release != nullptr)
        {
            command->proc_release(command);
        }
    };

    inline void command_do(Command* command)
    {
        NDBL_ASSERT(command->proc_do != nullptr, "proc_do should be defined" );
        command->proc_do(command);
    };

    inline void command_redo(Command* command)
    {
        // Some commands don't have a redo, because proc_redo and proc_do are implicitly the same thing.
        if( command->proc_redo != nullptr)
        {
            return command->proc_redo(command);
        }
        command_do(command);
    };

    inline void command_undo(Command* command)
    {
        NDBL_ASSERT(command->proc_undo != nullptr, "A command must be undoable! Write a simple procedure if needed." );
        command->proc_undo(command);
    };

    // Command declarations (see implementations in impl/ folder)

    Command make_cmd__connect(const Cmd_Payload__Link&);
    Command make_cmd__disconnect(const Cmd_Payload__Link&);
    Command make_cmd__text_undo_record(const Cmd_Data__Text_Undo_Record&);
    Command make_cmd__new_node(const Node_Init_Params&);

    struct Cmd_Data__Selection_Change
    {
        bdc::Array<View>     old_selection;
        bdc::Array<View>     new_selection;
    };

    Command make_cmd__selection_change(const Cmd_Data__Selection_Change&);
    Command make_cmd__set_node_view_position(const Cmd_Payload__Set_Node_View_Position&);
}


