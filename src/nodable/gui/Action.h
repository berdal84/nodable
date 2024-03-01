#pragma once
#include "core/Graph.h"
#include "fw/core/reflection/func_type.h"
#include "fw/gui/EventManager.h"

namespace ndbl
{
    struct CreateNodeActionPayload
    {
        const fw::func_type* node_signature;
        NodeType             node_type;
    };
    using CreateNodeAction = fw::TAction<CreateNodeActionPayload> ;
}