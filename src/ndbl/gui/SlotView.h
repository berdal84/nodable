#pragma once

#include "tools/core/reflection/Type.h"
#include "tools/gui/ImGuiEx.h"

#include "ndbl/core/Slot.h"
#include "ndbl/core/Property.h"
#include "types.h"
#include "tools/gui/geometry/Vec2.h"
#include "tools/gui/ViewState.h"


namespace ndbl
{
    class NodeView;

    enum ShapeType
    {
        ShapeType_NONE = 0,
        ShapeType_CIRCLE,
        ShapeType_RECTANGLE
    };

    struct SlotView
    {
        typedef tools::Vec2          Vec2;
        typedef tools::BoxShape2D    BoxShape2D;
        typedef tools::SpatialNode2D SpatialNode2D;
        typedef tools::string64      string64;
        typedef tools::ViewState     ViewState;
        typedef tools::TypeDescriptor TypeDescriptor;

        SlotView(
            Slot*,
            const Vec2& /* alignment */,
            ShapeType,
            size_t /* index */,
            const BoxShape2D* /* alignment_ref */
            );

        const size_t    index;
        const ShapeType shape_type;
        Vec2            direction; // cached
        Slot* const     slot;
        Vec2            alignment;
        const BoxShape2D* alignment_ref;

        void                  update(float dt);
        bool                  draw();
        string64              compute_tooltip() const;
        Node*                 node()const { return slot->node; }
        bool                  allows(SlotFlag flags) const { return slot->has_flags(flags); }
        SpatialNode2D&        spatial_node() { return shape().spatial_node(); }
        const SpatialNode2D&  spatial_node() const { return shape().spatial_node(); }
        BoxShape2D&           shape() { return _state.shape(); }
        const BoxShape2D&     shape() const { return _state.shape(); }
        ViewState&            state() { return _state; }
        const ViewState&      state() const { return _state; }
        const Property*       property()const { return slot->property; }
        const TypeDescriptor* property_type()const { return property() ? property()->get_type() : nullptr; }
        void                  update_direction_from_alignment();

    private:
        ViewState _state;
    };
}