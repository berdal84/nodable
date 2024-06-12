#pragma once

#include <observe/event.h>

#include "tools/core/reflection/type.h"
#include "tools/gui/ImGuiEx.h"

#include "ndbl/core/Slot.h"
#include "types.h"
#include "tools/core/geometry/Vec2.h"
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
            PoolID<NodeView> parent,
            Slot& slot,
            const tools::Vec2& align,
            ShapeType shape
            );

        bool                  draw() override;
        Property*             get_property()const;
        const tools::type*    get_property_type()const;
        tools::string64       compute_tooltip() const;
        PoolID<Node>          get_node()const;
        bool                  has_node_connected() const;
        Slot&                 slot()const;
        tools::Vec2           normal() const;
        PoolID<Node>          adjacent_node() const;
        bool                  is_this() const;
        bool                  allows(SlotFlag) const;
    private:
        u8_t                  m_index;
        ShapeType             m_shape;
        PoolID<NodeView>      m_parent;
        Slot&                 m_slot; // TODO: memory issue, use SlotRef instead
        tools::Vec2           m_align;
    };
}