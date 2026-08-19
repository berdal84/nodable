#pragma once

#include "tools/gui/geometry/Box_2D.h"
#include "tools/core/reflection/Type_Descriptor.h"
#include "tools/core/reflection/GETTERS_SETTERS.h"
#include "tools/gui/geometry/Vec2.h"
#include "tools/gui/View_Flags.h"

#include "ndbl/core/Node_Property.h"
#include "ndbl/core/Node_Slot.h"


namespace ndbl
{
    class Node_View;

    enum Shape_Type
    {
        Shape_Type_NONE = 0,
        Shape_Type_CIRCLE,
        Shape_Type_RECTANGLE
    };

    struct Node_Slot_View
    {   
        Node_Slot*                slot;
        u32_t                     index;
        Shape_Type                shape_type;
        tools::Vec2               direction; // cached
        tools::Vec2               alignment_pivot;
        const tools::Box_2D*      alignment_ref;
        tools::Box_2D             shape;
        tools::View_Flags         flags;

        GETTER( Node*            , node      , slot->node )
        GETTER( Node_Property*   , property  , slot->property )

        inline bool                          allows(Node_Slot::Flag flags) const { return HAS_FLAGS(slot->flags, flags); }
        inline const tools::Type_Descriptor* property_type()const                { return property() ? property()->type : nullptr; }
    };

    void        nodeslotview_init(Node_Slot_View*, Node_Slot*, const tools::Vec2& /* alignment */,
                                  Shape_Type, u32_t /* index */, const tools::Box_2D* /* alignment_ref */);
    void        nodeslotview_update(Node_Slot_View*, float dt);
    bool        nodeslotview_draw(Node_Slot_View*);
    bdc::String nodeslotview_compute_tooltip(const Node_Slot_View*);
    void        nodeslotview_update_direction_from_alignment(Node_Slot_View*);
}