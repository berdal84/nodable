#pragma once

#include "bdc/Types.hpp"
#include "ndbl/core/Event.h"
#include "ndbl/core/reflection/Type_Descriptor.h"
#include "ndbl/core/Graph.h"
#include "geometry/Vec2.h"
#include "View.h"

namespace ndbl
{
    // forward declaration
    struct Node_View;
    struct Node_Slot;
    struct Node;
    struct Graph;

    struct Event_Data__Selection
    {
        View_Selection selected_items = {};
    };

    struct Event_Data__Create_Node
    {
        Node_State          node_state;
        Node_Slot_View*     active_slotview;    // The slot view being dragged.
        Vec2                desired_screen_pos; // The desired position for the new node view
        Type_Descriptor*    function_type;
    };

}// namespace ndbl
