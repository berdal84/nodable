#pragma once

#include <observe/event.h>

#include "tools/core/reflection/Type.h"
#include "tools/gui/ImGuiEx.h"

#include "ndbl/core/Slot.h"
#include "types.h"
#include "tools/gui/geometry/Vec2.h"
#include "tools/gui/View.h"


namespace ndbl
{
    class NodeView;

    enum ShapeType
    {
        ShapeType_CIRCLE,
        ShapeType_RECTANGLE
    };

    class SlotView : public tools::View
    {
    public:
        SlotView(
            Slot* slot,
            const tools::Vec2& align,
            ShapeType shape,
            size_t index
            );

        bool                  draw() override;
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

    private:
        size_t                m_index;
        ShapeType             m_shape;
        Slot*                 m_slot;
        tools::Vec2           m_align;
    };
}