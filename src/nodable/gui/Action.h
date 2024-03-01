#pragma once
#include "Event.h"
#include "core/Graph.h"
#include "fw/core/reflection/func_type.h"
#include "fw/gui/Action.h"

namespace ndbl
{
    using fw::SimpleAction;
    using fw::CustomAction;

    // Actions specific to Nodable, more actions defined in framework's Action.h

    using Action_DeleteNode      = SimpleAction<EventID_REQUEST_DELETE_NODE>;
    using Action_ArrangeNode     = SimpleAction<EventID_REQUEST_ARRANGE_HIERARCHY>;
    using Action_ToggleFolding   = SimpleAction<EventID_REQUEST_TOGGLE_FOLDING>;
    using Action_SelectNext      = SimpleAction<EventID_REQUEST_SELECT_SUCCESSOR>;
    using Action_Isolate         = SimpleAction<EventID_REQUEST_TOGGLE_ISOLATE_SELECTION>;
    using Action_SelectionChange = SimpleAction<EventID_NODE_SELECTION_CHANGE>;
    using Action_MoveGraph       = SimpleAction<EventID_REQUEST_MOVE_SELECTION>;
    using Action_FrameGraph      = SimpleAction<EventID_REQUEST_FRAME_SELECTION>;
    using Action_CreateBlock     = CustomAction<Event_CreateBlock>;
    using Action_CreateNode      = CustomAction<Event_CreateNode> ;
}