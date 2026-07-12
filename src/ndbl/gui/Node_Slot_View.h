#pragma once

#include "tools/core/reflection/Type_Descriptor.h"

#include "ndbl/core/Node_Slot.h"
#include "ndbl/core/Node_Property.h"
#include "tools/gui/geometry/Vec2.h"
#include "tools/gui/View_State.h"


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
        Node_Slot_View(
                Node_Slot*,
                const tools::Vec2&   /* alignment */,
                Shape_Type,
                size_t               /* index */,
                const tools::Box_2D* /* alignment_ref */
            );

        const size_t                    index;
        const Shape_Type                shape_type;
        tools::Vec2                     direction; // cached
        Node_Slot* const                slot;
        tools::Vec2                     alignment;
        const tools::Box_2D*            alignment_ref;
        tools::View_State               state;
        tools::Box_2D                   shape;

        // shorthands
        
        Node*                           node()const { return slot->node; }
        bool                            allows(Node_Slot::Flag flags) const { return slot->has_flags(flags); }
        tools::Spatial_Node*            spatial_node() { return shape.spatial_node(); }
        const tools::Spatial_Node*      spatial_node() const { return shape.spatial_node(); }
        const Node_Property*            property()const { return slot->property; }
        const tools::Type_Descriptor*   property_type()const { return property() ? property()->type : nullptr; }
    };

    void                            nodeslotview_update(Node_Slot_View*, float dt);
    bool                            nodeslotview_draw(Node_Slot_View*);
    tools::String_64                nodeslotview_compute_tooltip(const Node_Slot_View*);
    void                            nodeslotview_update_direction_from_alignment(Node_Slot_View*);
}