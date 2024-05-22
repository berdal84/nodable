#pragma once

#include "fw/core/reflection/func_type.h"
#include "fw/gui/Action.h"
#include "nodable/core/Graph.h"
#include "Event.h"

namespace ndbl
{
    using fw::Action;
    using fw::Event;

    // Actions specific to Nodable, more actions defined in framework's Action.h

    // 1) Basic actions (simple events)

    using Action_DeleteNode      = Action<Event_DeleteNode>;
    using Action_ArrangeNode     = Action<Event_ArrangeNode>;
    using Action_ToggleFolding   = Action<Event_ToggleFolding>;
    using Action_SelectNext      = Action<Event_SelectNext>;
    using Action_ToggleIsolate   = Action<Event_ToggleIsolationFlags>;
    using Action_SelectionChange = Action<Event_SelectionChange>;
    using Action_MoveGraph       = Action<Event_MoveSelection>;

    // 2) Advanced actions (custom events)

    using Action_FrameSelection  = Action<Event_FrameSelection>;
    using Action_CreateNode      = Action<Event_CreateNode>;
}