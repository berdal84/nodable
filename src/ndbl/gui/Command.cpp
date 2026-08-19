#include "Command.h"
#include "core/Graph.h"
#include "gui/File.h"
#include "gui/Graph_View.h"
#include "gui/Nodable.h"
#include "gui/View.h"
#include <cstring>

//-----------------------------------------------------------------------------

void ndbl::command_release(Command* command)
{
    if( command->proc_release != nullptr)
    {
        command->proc_release(command);
    }
}

void ndbl::command_do(Command* command)
{
    command->proc_do(command);
}

void ndbl::command_redo(Command* command)
{

    if( command->proc_redo != nullptr)
    {
        return command->proc_redo(command);
    }
    
    command->proc_do(command);
}

void ndbl::command_undo(Command* command)
{
    command->proc_undo(command);
};

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_connect(Command_Data__Link link)
{
    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_connect_do;
    command.proc_undo   = command_connect_undo;
    command.link        = link;
    command.description = "Connect two slots";

    return command;
}

void ndbl::command_connect_do(Command* command)
{
    graph_connect( command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::command_connect_undo(Command* command)
{
    graph_disconnect(command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_disconnect(Command_Data__Link link)
{
    Command command;
    command.type        = Command_Type_DISCONNECT;
    command.proc_do     = command_disconnect_do;
    command.proc_undo   = command_disconnect_undo;
    command.link        = link;
    command.description = "Disconnect two slots";

    return command;
}

void ndbl::command_disconnect_do(Command* command)
{
    graph_disconnect( command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::command_disconnect_undo(Command* command)
{
    graph_connect(command->link.tail, command->link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_text_undo_record(Command_Data__Text_Undo_Record text_undo_record)
{
    Command command;
    command.type                = Command_Type_TEXT_UNDO_RECORD;
    command.proc_do             = command_text_undo_record_do;
    command.proc_undo           = command_text_undo_record_undo;
    command.text_undo_record    = text_undo_record;
    command.description         = "Wraps a TextEditor's Undo Record";

    return command;
}

void ndbl::command_text_undo_record_do(Command* command)
{
    TextEditor* text_editor = &app_state()->current_file->view.text_editor;
    command->text_undo_record.undo_record->Redo(text_editor);
}

void ndbl::command_text_undo_record_undo(Command* command)
{
    TextEditor* text_editor = &app_state()->current_file->view.text_editor;
    command->text_undo_record.undo_record->Undo(text_editor);
}

//-----------------------------------------------------------------------------

ndbl::Command ndbl::command_new_node(Command_Data data)
{
    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_new_node_do;
    command.proc_undo   = command_new_node_undo;
    command.data        = data;
    command.description = "Connect two slots";

    return command;
}

void ndbl::command_new_node_do(Command* command)
{
    Graph*  graph       = app_state()->current_file->graph;
    auto    node_state  = static_cast<Node_State*>(command->data.data1);

    if ( !graph_root(graph) )
    {
        TOOLS_LOG(tools::Verbosity_Error, "Nodable", "Unable to create_new primary_child, no root found on this graph.\n");
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
                new_node->suffix = Token::s_end_of_line;
                break;
            case Node_Type_VARIABLE:
            case Node_Type_RETURN:
            case Node_Type_EMPTY_INSTRUCTION:
                new_node->suffix = Token::s_end_of_instruction;
                break;
            case Node_Type_LITERAL:
            case Node_Type_FUNCTION:
                break;
            default:
                TOOLS_UNREACHABLE("Unexpected Node_Type: %i\n", node_state->type );
        }
    }

    command->data.data2 = new_node;
}

void ndbl::command_new_node_undo(Command* command)
{
    auto node = static_cast<Node*>(command->data.data2);
    if( node != nullptr)
        graph_find_and_destroy_node( app_state()->current_file->graph, node);
}

//-----------------------------------------------------------------------------

namespace ndbl
{
    inline void command_selection_change_release(Command *command)
    {
       delete command->selection_change.old_selection;
       delete command->selection_change.new_selection;
       memset(command, 0, sizeof(Command));
    }
}

ndbl::Command ndbl::command_selection_change(const View_Selection* _selection)
{
    Command cmd{};

    cmd.type            = Command_Type_SELECTION_CHANGE;
    cmd.proc_do         = command_selection_change_do;
    cmd.proc_redo       = cmd.proc_do;
    cmd.proc_undo       = command_selection_change_undo;
    cmd.proc_release    = command_selection_change_release;

    auto old_selection = bdc::memory_new<View_Selection>();
    view_selection_add(old_selection, app_state()->current_file->view.graph_view->selection.items);

    auto new_selection = bdc::memory_new<View_Selection>();
    view_selection_add(new_selection, _selection->items);

    cmd.selection_change = {
        old_selection,
        new_selection,
    };
    return cmd;
}

void ndbl::command_selection_change_do(Command *command)
{
    auto selection = &app_state()->current_file->view.graph_view->selection;
    view_selection_clear( selection );
    view_selection_add( selection, command->selection_change.new_selection->items);
}

void ndbl::command_selection_change_undo(Command *command)
{
    auto selection = &app_state()->current_file->view.graph_view->selection;
    view_selection_clear( selection );
    view_selection_add( selection, command->selection_change.old_selection->items);
}

//-----------------------------------------------------------------------------
