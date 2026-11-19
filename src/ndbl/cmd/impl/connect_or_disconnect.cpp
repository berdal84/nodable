#include "ndbl/cmd/Command.h"

#include "ndbl/File.h"
#include "ndbl/Graph_View.h"
#include "ndbl/Graph.h"
#include "ndbl/Nodable.h"
#include "ndbl/View.h"

namespace ndbl 
{

void _command_connect_do(Command*);    // shared by connect and disconnect
void _command_disconnect_do(Command*); // shared by connect and disconnect

Command make_cmd__connect(const Cmd_Payload__Link& link)
{
    NDBL_ASSERT(link.head != link.tail);

    Command cmd{};

    cmd.proc_do         = _command_connect_do;
    cmd.proc_undo       = _command_disconnect_do;    
    cmd.payload.link    = link;
    cmd.description     = "Connects two slots";

    // TODO: it does not feel right do do this here, rather have an utility procedure that make_cmd_connect's user will call ahead of time.
    if ( link.tail->order() == Node_Slot::Flag_ORDER_2ND )
    {
        if ( link.head->order() == Node_Slot::Flag_ORDER_2ND )
        {
            NDBL_LOG( NDBL_ERROR, "make_cmd_connect", "Unable to connect incompatible edges\n");
            return {};
        }
        NDBL_DEBUG_LOG( NDBL_DIAG, "make_cmd_connect", "Swapping edges to try to connect them\n");
        std::swap(cmd.payload.link.tail, cmd.payload.link.head);
    }

    return cmd;
}

Command make_cmd__disconnect(const Cmd_Payload__Link& link)
{
    NDBL_ASSERT(link.head != link.tail);

    Command cmd{};
    cmd.proc_do         = _command_disconnect_do;
    cmd.proc_undo       = _command_connect_do;
    cmd.payload.link    = link;
    cmd.description     = "Disconnects two slots";

    return cmd;
}

void _command_connect_do(Command* cmd)
{
    graph_connect( cmd->payload.link.tail, cmd->payload.link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void _command_disconnect_do(Command* cmd)
{
    graph_disconnect(cmd->payload.link.tail, cmd->payload.link.head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

} // namespace ndbl