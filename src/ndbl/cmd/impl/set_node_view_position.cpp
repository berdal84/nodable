#include "ndbl/cmd/Command.h"

#include "ndbl/Nodable.h"
#include "ndbl/Graph.h"
#include "ndbl/Node.h"
#include "ndbl/Node_View.h"
#include "ndbl/geometry/Spatial_Node.h"
#include "ndbl/File.h"

namespace ndbl 
{

void _set_node_view_position_do(Command*);

Command make_cmd__set_node_view_position(const Cmd_Payload__Set_Node_View_Position& data)
{
    Command cmd{};

    cmd.proc_do     = _set_node_view_position_do;
    cmd.proc_redo   = cmd.proc_do;
    cmd.description = "Set Node view position";
    cmd.payload.set_node_view_position = data;

    return cmd;
}

void _set_node_view_position_do(Command* cmd)
{
    Cmd_Payload__Set_Node_View_Position& payload = cmd->payload.set_node_view_position;
    Graph* graph = app_state()->current_file->graph;
    Node*  node  = graph_get_node(graph, payload.node_id);
    spatialnode_set_position(node->view->shape.spatial_node, { payload.x, payload.y }, payload.space);
}

} // namespace ndbl