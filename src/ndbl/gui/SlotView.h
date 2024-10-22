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

        const size_t index;
        const ShapeType shape;
        Vec2         _alignment;
        Vec2         direction; // cached
        ViewState    state;
        Slot* const  _slot;
        const BoxShape2D* alignment_ref;

        void         update(float dt);
        bool         draw();
        string64     compute_tooltip() const;
        inline Node* node()const { return _slot->node(); }
        inline Slot& slot() const { return *_slot; }
        inline bool  allows(SlotFlag flags) const { return _slot->has_flags(flags); }
        void         set_alignment(const tools::Vec2);

        // aliases
        inline SpatialNode2D*       xform() { return &state.box.xform; }
        inline const SpatialNode2D* xform() const { return &state.box.xform; }
        inline BoxShape2D*          box() { return &state.box; }
        inline const BoxShape2D*    box() const { return &state.box; }
        inline const Property*      property()const { return _slot->property(); }
        inline const TypeDescriptor* property_type()const { return property() ? property()->get_type() : nullptr; }

        void _update_direction_from_alignment();
        void _update_size_from_shape();
    };
}