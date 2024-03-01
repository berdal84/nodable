#pragma once
#include "Event.h"
#include "core/Graph.h"
#include "fw/core/reflection/func_type.h"
#include "fw/gui/Action.h"

namespace ndbl
{
    using fw::BasicAction;
    using fw::CustomAction;

    // Actions specific to Nodable, more actions defined in framework's Action.h

    using Action_DeleteNode      = BasicAction<EventID_REQUEST_DELETE_NODE>;
    using Action_ArrangeNode     = BasicAction<EventID_REQUEST_ARRANGE_HIERARCHY>;
    using Action_ToggleFolding   = BasicAction<EventID_REQUEST_TOGGLE_FOLDING>;
    using Action_SelectNext      = BasicAction<EventID_REQUEST_SELECT_SUCCESSOR>;
    using Action_Isolate         = BasicAction<EventID_REQUEST_TOGGLE_ISOLATE_SELECTION>;
    using Action_SelectionChange = BasicAction<EventID_NODE_SELECTION_CHANGE>;
    using Action_MoveGraph       = BasicAction<EventID_REQUEST_MOVE_SELECTION>;
    using Action_FrameGraph      = BasicAction<EventID_REQUEST_FRAME_SELECTION>;
    using Action_CreateBlock     = CustomAction<Event_CreateBlock>;
    using Action_CreateNode      = CustomAction<Event_CreateNode>;
}