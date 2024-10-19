#pragma once

#include <observe/event.h>

#include "tools/core/reflection/Type.h"
#include "tools/gui/ImGuiEx.h"

#include "ndbl/core/Slot.h"
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

    class SlotView
    {
    public:
        SlotView(
            Slot* slot,
            const tools::Vec2& align,
            ShapeType shape,
            size_t index
            );

        bool                  draw();
        Property*             property()const;
        const tools::TypeDescriptor* property_type()const;
        tools::string64       compute_tooltip() const;
        Node*                 node()const;
        bool                  has_node_connected() const;
        Slot&                 slot()const;
        const tools::Vec2&    alignment() const;
        tools::Vec2           normal() const;
        Node*                 adjacent_node() const;
        bool                  is_this() const;
        bool                  is_hovered() const;
        bool                  allows(SlotFlag) const;
        size_t                index() const;
        ShapeType             shape() const;
        void                  set_visible(bool b) { m_view_state.visible = b; }
        void                  set_align(const tools::Vec2 vec2);
        void                  set_shape(ShapeType type);

        // aliases

        tools::ViewState*           state();
        tools::SpatialNode2D*       xform() { return &m_view_state.box.xform; }
        const tools::SpatialNode2D* xform() const { return &m_view_state.box.xform; }
        tools::BoxShape2D*          box() { return &m_view_state.box; }
        const tools::BoxShape2D*    box() const { return &m_view_state.box; }

    private:
        size_t                m_index;
        ShapeType             m_shape;
        Slot*                 m_slot;
        tools::Vec2           m_align;
        tools::ViewState      m_view_state;
    };
}