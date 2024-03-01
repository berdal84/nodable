#pragma once
#include "Event.h"
#include "core/Graph.h"
#include "fw/core/reflection/func_type.h"
#include "fw/gui/Action.h"

namespace ndbl
{

    enum class NodeActionType
    {
        DELETE,
        ARRANGE
    };
    using NodeAction  = fw::TAction<EventID_REQUEST_DELETE_NODE, NodeActionType>;

    struct CreateNodeActionPayload
    {
        NodeType             node_type;
        const fw::func_type* node_signature{};
        SlotView*            dragged_slot{};
        Graph*               graph{};
        ImVec2               node_view_local_pos;
    };
    using CreateNodeAction = fw::TAction<EventID_REQUEST_CREATE_NODE, CreateNodeActionPayload> ;

    using CreateBlockAction = fw::TAction<EventID_REQUEST_CREATE_BLOCK, NodeType>;
}