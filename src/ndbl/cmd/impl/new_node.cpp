#include "ndbl/cmd/Command.h"

#include "ndbl/File.h"
#include "ndbl/Graph_View.h"
#include "ndbl/Graph.h"
#include "ndbl/Nodable.h"
#include "ndbl/View.h"

namespace ndbl 
{

void _new_node__do(Command*);
void _new_node__undo(Command*);

Command make_cmd__new_node(const Node_Init_Params& node_init_params)
{
    Command cmd{};

    cmd.proc_do     = _new_node__do;
    cmd.proc_undo   = _new_node__undo;
    cmd.payload.new_node.node_init_params = node_init_params,
    cmd.description = "Connect two slots";

    return cmd;
}

void _new_node__do(Command* cmd)
{
    Graph* graph = app_state()->current_file->graph;
    graph_new_node(graph, cmd->payload.new_node.node_init_params );
}

void _new_node__undo(Command* cmd)
{
    Graph* graph = app_state()->current_file->graph;
    graph_delete(graph, cmd->payload.new_node.node_init_params.id );
}

} // namespace ndbl