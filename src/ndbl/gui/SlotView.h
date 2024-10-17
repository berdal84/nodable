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
        Property*             get_property()const;
        const tools::TypeDescriptor*get_property_type()const;
        tools::string64       compute_tooltip() const;
        Node*                 get_node()const;
        bool                  has_node_connected() const;
        Slot&                 get_slot()const;
        const tools::Vec2&    get_align() const;
        tools::Vec2           get_normal() const;
        Node*                 adjacent_node() const;
        bool                  is_this() const;
        bool                  allows(SlotFlag) const;
        size_t                get_index() const;
        ShapeType             get_shape() const;
        tools::Vec2           get_pos(tools::Space_ space) const;
        tools::ViewState*     state_handle();
        void                  set_visible(bool b) { m_state.visible = b; }

        void set_pos(tools::Vec2 vec2);

        void set_size(tools::Vec2 vec2);

        bool is_hovered() const;

    private:
        size_t                m_index;
        ShapeType             m_shape;
        Slot*                 m_slot;
        tools::Vec2           m_align;
        tools::ViewState      m_state;
    };
}