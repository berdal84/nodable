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

Command make_cmd_connect(Cmd_Link link)
{


    NDBL_ASSERT(link.head != link.tail);
    if ( link.tail->order() == Node_Slot::Flag_ORDER_2ND )
    {
        if ( link.head->order() == Node_Slot::Flag_ORDER_2ND )
        {
            NDBL_LOG( NDBL_ERROR, "command_connect", "Unable to connect incompatible edges\n");
            return {};
        }
        NDBL_DEBUG_LOG( NDBL_DIAG, "command_connect", "Swapping edges to try to connect them\n");
        std::swap(link.tail, link.head);
    }

    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_connect_do;
    command.proc_undo   = command_connect_undo;
    command.data.link   = link;
    command.description = "Connect two slots";

    return command;
}

void command_connect_do(Command* command)
{
    graph_connect( command->data.link.tail, command->data.link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void command_connect_undo(Command* command)
{
    graph_disconnect(command->data.link.tail, command->data.link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

Command make_cmd_disconnect(Cmd_Link link)
{
    Command command;
    command.type        = Command_Type_DISCONNECT;
    command.proc_do     = command_disconnect_do;
    command.proc_undo   = command_disconnect_undo;
    command.data.link   = link;
    command.description = "Disconnect two slots";

    return command;
}

void command_disconnect_do(Command* command)
{
    graph_disconnect( command->data.link.tail, command->data.link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void command_disconnect_undo(Command* command)
{
    graph_connect(command->data.link.tail, command->data.link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

//-----------------------------------------------------------------------------

Command make_cmd_text_undo_record(const Cmd_Text_Undo_Record& text_undo_record)
{
    Command command;
    command.type                = Command_Type_TEXT_UNDO_RECORD;
    command.proc_do             = command_text_undo_record_do;
    command.proc_undo           = command_text_undo_record_undo;
    command.proc_release        = command_text_undo_record_release;
    command.data.text_undo_record    = memory_new<Cmd_Text_Undo_Record>(allocator, text_undo_record);
    command.description         = "Wraps a Text_Editor's Undo Record";

    return command;
}

void command_text_undo_record_do(Command* command)
{
    Text_Editor* text_editor = &app_state()->current_file->view.text_editor;
    command->data.text_undo_record->undo_record.Redo(text_editor);
}

void command_text_undo_record_undo(Command* command)
{
    Text_Editor* text_editor = &app_state()->current_file->view.text_editor;
    command->data.text_undo_record->undo_record.Undo(text_editor);
}

void command_text_undo_record_release(Command* command)
{
    memory_delete(command->data.text_undo_record);
    command->data.text_undo_record = nullptr;
}

//-----------------------------------------------------------------------------

Command make_cmd_new_node(const Node_Init_Params& node_init_params)
{
    Command command;
    command.type        = Command_Type_CONNECT;
    command.proc_do     = command_new_node_do;
    command.proc_undo   = command_new_node_undo;
    command.data.new_node.node_init_params = node_init_params;
    command.description = "Connect two slots";

    return command;
}

void command_new_node_do(Command* command)
{
    auto&  data  = command->data.new_node;
    Graph* graph = app_state()->current_file->graph;

    graph_new_node(graph, data.node_init_params );
}

void command_new_node_undo(Command* command)
{
    auto&  data  = command->data.new_node;
    Graph* graph = app_state()->current_file->graph;

    graph_delete(graph, data.node_init_params.id);
}

//-----------------------------------------------------------------------------

NDBL_COMMAND(selection_change, const Cmd_Selection_Change& selection_change)
{
    Command cmd;

    cmd.type                    = Command_Type_SELECTION_CHANGE;
    cmd.proc_do                 = command_selection_change_do;
    cmd.proc_redo               = cmd.proc_do;
    cmd.proc_undo               = command_selection_change_undo;
    cmd.proc_release            = command_selection_change_release;
    cmd.data.selection_change   = cmd_selection_change_copy(memory_new<Cmd_Selection_Change>(), &selection_change);

    return cmd;
}

NDBL_COMMAND_RELEASE(selection_change, command)
{
    memory_delete( command->data.selection_change );    
    *command = {};
}

NDBL_COMMAND_DO(selection_change, command)
{
    auto& selection = app_state()->current_file->view.graph_view->selection;
    viewselection_clear( selection );
    viewselection_add( selection, command->data.selection_change->new_selection);
}

NDBL_COMMAND_UNDO(selection_change, command)
{
    auto& selection = app_state()->current_file->view.graph_view->selection;
    viewselection_clear( selection );
    viewselection_add( selection, command->data.selection_change->old_selection);
}

//-----------------------------------------------------------------------------

NDBL_COMMAND(set_node_view_position, const Set_Node_View_Position& data)
{
    Command cmd;

    cmd.type                        = Command_Type_SELECTION_CHANGE;
    cmd.proc_do                     = command_set_node_view_position_do;
    cmd.proc_redo                   = cmd.proc_do;
    cmd.data.set_node_view_position = data;

    return cmd;
}

NDBL_COMMAND_DO(set_node_view_position, command)
{
    Set_Node_View_Position& data = command->data.set_node_view_position;
    Graph* graph = app_state()->current_file->graph;
    Node*  node  = graph_get_node(graph, data.node_id);
    spatialnode_set_position(node->view->shape.spatial_node, { data.x, data.y }, data.space);
}

//-----------------------------------------------------------------------------

} // namespace ndbl