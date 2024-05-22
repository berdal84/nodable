#pragma once

#include <observe/event.h>

#include "fw/core/reflection/type.h"
#include "fw/gui/ImGuiEx.h"

#include "nodable/core/Slot.h"
#include "types.h"


namespace ndbl
{
    enum Side
    {
        Side_TOP = 0,
        Side_BOTTOM,
        Side_LEFT,
        Side_RIGHT,
        Side_COUNT
    };

    class SlotView
    {
    public:
        SlotView( Slot& _slot, fw::Vec2 _alignment );

        Property*             get_property()const;
        const fw::type*       get_property_type()const;
        bool                  has_node_connected() const;
        fw::Vec2              alignment() const;
        Slot&                 slot()const;
        PoolID<Node>          get_node()const;
        fw::Vec2              position()const;
        fw::Rect              get_rect()const;
        PoolID<Node>          adjacent_node() const;
        bool                  is_this() const;
        bool                  allows(SlotFlag) const;
        static SlotView*      get_dragged() { return s_dragged; }
        static SlotView*      get_focused() { return s_dragged; }
        static SlotView*      get_hovered() { return s_hovered; }
        static bool           is_dragging() { return s_dragged; }
        static void           drop_behavior(bool& require_new_node, bool _enable_edition);
        static void           reset_dragged(SlotView * slot = nullptr) { s_dragged = slot; }
        static void           reset_focused(SlotView * slot = nullptr) { s_focused = slot; }
        static void           reset_hovered(SlotView * slot = nullptr) { s_hovered = slot; }
        static void           behavior(SlotView&, bool _readonly);
        static void           draw_slot_circle( ImDrawList* _draw_list, SlotView& _view,  fw::Vec2 _position, bool _readonly );
        static void           draw_slot_rectangle(ImDrawList* _draw_list, SlotView& _view, fw::Rect _rect, bool _readonly);

    private:
        Slot&                 m_slot;
        fw::Vec2 m_alignment;
        static SlotView*      s_focused;
        static SlotView*      s_dragged;
        static SlotView*      s_hovered;
        static std::string get_tooltip( SlotView &view );
    };
}