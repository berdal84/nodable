#include "Command.h"

#include <cstring>

#include "File.h"
#include "Graph_View.h"
#include "Graph.h"
#include "Nodable.h"
#include "View.h"

namespace ndbl 
{

//-----------------------------------------------------------------------------

void command_release(Command* command)
{
    if( command->proc_release != nullptr)
    {
        command->proc_release(command);
    }
}

void command_do(Command* command)
{
    command->proc_do(command);
}

void command_redo(Command* command)
{

    if( command->proc_redo != nullptr)
    {
        return command->proc_redo(command);
    }
    
    command->proc_do(command);
}

void command_undo(Command* command)
{
    command->proc_undo(command);
};

//-----------------------------------------------------------------------------

Command new_command_connect(const Command_Data__Link& link)
{
    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_connect_do;
    command.proc_undo   = command_connect_undo;
    command.link        = link;
    command.description = "Connect two slots";

    NDBL_ASSERT(command.link.head != command.link.tail);
    if ( command.link.tail->order() == Node_Slot::Flag_ORDER_2ND )
    {
        if ( command.link.head->order() == Node_Slot::Flag_ORDER_2ND )
        {
            NDBL_LOG( NDBL_ERROR, "command_connect", "Unable to connect incompatible edges\n");
            return {};
        }
        NDBL_DEBUG_LOG( NDBL_DIAG, "command_connect", "Swapping edges to try to connect them\n");
        std::swap(command.link.tail, command.link.head);
    }

    return command;
}

void command_connect_do(Command* command)
{
    graph_connect( command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void command_connect_undo(Command* command)
{
    graph_disconnect(command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

Command new_command_disconnect(const Command_Data__Link& link)
{
    Command command;
    command.type        = Command_Type_DISCONNECT;
    command.proc_do     = command_disconnect_do;
    command.proc_undo   = command_disconnect_undo;
    command.link        = link;
    command.description = "Disconnect two slots";

    return command;
}

void command_disconnect_do(Command* command)
{
    graph_disconnect( command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void command_disconnect_undo(Command* command)
{
    graph_connect(command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

Command new_command_text_undo_record(const Command_Data__Text_Undo_Record& text_undo_record)
{
    Command command;
    command.type                = Command_Type_TEXT_UNDO_RECORD;
    command.proc_do             = command_text_undo_record_do;
    command.proc_undo           = command_text_undo_record_undo;
    command.proc_release        = command_text_undo_record_release;
    command.text_undo_record    = memory_new<Command_Data__Text_Undo_Record>(allocator, text_undo_record);
    command.description         = "Wraps a Text_Editor's Undo Record";

    return command;
}

void command_text_undo_record_do(Command* command)
{
    Text_Editor* text_editor = &app_state()->current_file->view.text_editor;
    command->text_undo_record->undo_record.Redo(text_editor);
}

void command_text_undo_record_undo(Command* command)
{
    Text_Editor* text_editor = &app_state()->current_file->view.text_editor;
    command->text_undo_record->undo_record.Undo(text_editor);
}

void command_text_undo_record_release(Command* command)
{
    memory_delete(command->text_undo_record);
    *command = {};
}

//-----------------------------------------------------------------------------

Command new_command_new_node(const Command_Data& data)
{
    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_new_node_do;
    command.proc_undo   = command_new_node_undo;
    command.data        = data;
    command.description = "Connect two slots";

    return command;
}

void command_new_node_do(Command* command)
{
    Graph*  graph       = app_state()->current_file->graph;
    auto    node_state  = static_cast<Node_State*>(command->data.data1);

    if ( !graph_root(graph) )
    {
        NDBL_LOG(Verbosity_Error, "Nodable", "Unable to create_new primary_child, no root found on this graph.\n");
        return;
    }

    Scope*  root_scope = graph_root_scope(graph);                 
    Node*   new_node   = graph_create_node(graph, node_state, root_scope );

    if ( node_state->user_created )
    {
        switch ( node_state->type )
        {
            case Node_Type_IF_ELSE:
            case Node_Type_FOR_LOOP:
            case Node_Type_WHILE_LOOP:
            case Node_Type_SCOPE:
            case Node_Type_ROOT:
                new_node->suffix = token_end_of_line;
                break;
            case Node_Type_VARIABLE:
            case Node_Type_RETURN:
            case Node_Type_EMPTY_INSTRUCTION:
                new_node->suffix = token_end_of_instruction;
                break;
            case Node_Type_LITERAL:
            case Node_Type_FUNCTION:
                break;
            default:
                NDBL_UNREACHABLE("Unexpected Node_Type: %i\n", node_state->type );
        }
    }

    command->data.data2 = new_node;
}

void command_new_node_undo(Command* command)
{
    auto node = static_cast<Node*>(command->data.data2);
    if( node != nullptr)
        graph_find_and_destroy_node( app_state()->current_file->graph, node);
}

//-----------------------------------------------------------------------------

Command new_command_selection_change(const Array<View>& _selection) // TODO: we should not take a View_Selection as input arg, a const Array<View>& is enough
{
    Command cmd;

    cmd.type             = Command_Type_SELECTION_CHANGE;
    cmd.proc_do          = command_selection_change_do;
    cmd.proc_redo        = cmd.proc_do;
    cmd.proc_undo        = command_selection_change_undo;
    cmd.proc_release     = command_selection_change_release;
    cmd.selection_change = memory_new<Command_Data__Selection_Change>();
    
    cmd.selection_change->old_selection = array_copy( app_state()->current_file->view.graph_view->selection.items );
    cmd.selection_change->new_selection = array_copy( _selection );

    return cmd;
}

void command_selection_change_release(Command *command)
{
    memory_delete( command->selection_change );    
    *command = {};
}

void command_selection_change_do(Command *command)
{
    auto& selection = app_state()->current_file->view.graph_view->selection;
    viewselection_clear( selection );
    viewselection_add( selection, command->selection_change->new_selection);
}

void command_selection_change_undo(Command *command)
{
    auto& selection = app_state()->current_file->view.graph_view->selection;
    viewselection_clear( selection );
    viewselection_add( selection, command->selection_change->old_selection);
}

//-----------------------------------------------------------------------------

} // namespace ndbl