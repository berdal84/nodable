#pragma once

#include <observe/event.h>

#include "tools/core/reflection/type.h"
#include "tools/gui/ImGuiEx.h"

#include "ndbl/core/Slot.h"
#include "types.h"


namespace ndbl
{
    class SlotView
    {
    public:
        SlotView( Slot& _slot, tools::Vec2 _alignment );

        Property*             get_property()const;
        const tools::type*    get_property_type()const;
        std::string           get_tooltip() const;
        PoolID<Node>          get_node()const;
        tools::Rect           get_rect()const;
        bool                  has_node_connected() const;
        tools::Vec2           alignment() const;
        Slot&                 slot()const;
        tools::Vec2           position()const;
        PoolID<Node>          adjacent_node() const;
        bool                  is_this() const;
        bool                  allows(SlotFlag) const;

        static void           draw_slot_circle(ImDrawList* _draw_list, SlotView& _view, const tools::Vec2& _position );
        static void           draw_slot_rectangle(ImDrawList* _draw_list, SlotView& _view, const tools::Rect& _rect );
    private:
        Slot&                 m_slot;
        tools::Vec2           m_alignment;
    };
}