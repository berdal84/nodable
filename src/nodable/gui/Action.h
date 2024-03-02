#pragma once
#include "Event.h"
#include "core/Graph.h"
#include "fw/core/reflection/func_type.h"
#include "fw/gui/Action.h"

namespace ndbl
{
    using fw::Action;
    using fw::CustomAction;

    // Actions specific to Nodable, more actions defined in framework's Action.h

    using Action_DeleteNode      = Action<EventID_REQUEST_DELETE_NODE>;
    using Action_ArrangeNode     = Action<EventID_REQUEST_ARRANGE_HIERARCHY>;
    using Action_ToggleFolding   = Action<EventID_REQUEST_TOGGLE_FOLDING>;
    using Action_SelectNext      = Action<EventID_REQUEST_SELECT_SUCCESSOR>;
    using Action_ToggleIsolate   = Action<EventID_REQUEST_TOGGLE_ISOLATE>;
    using Action_SelectionChange = Action<EventID_NODE_SELECTION_CHANGE>;
    using Action_MoveGraph       = Action<EventID_REQUEST_MOVE_SELECTION>;
    using Action_FrameSelection  = CustomAction<Event_FrameSelection>;
    using Action_CreateBlock     = CustomAction<Event_CreateBlock>;
    using Action_CreateNode      = CustomAction<Event_CreateNode>;
}