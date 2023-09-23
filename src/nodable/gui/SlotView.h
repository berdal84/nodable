#pragma once
#include "observe/event.h"
#include "fw/gui/ImGuiEx.h"
#include "core/Slot.h"
#include "types.h"

namespace ndbl
{
    enum class Side
    {
        Top,
        Bottom,
        Left,
        Right
    };

    class SlotView
    {
    public:
        SlotView();
        SlotView(Slot&, Side);

        Property*             get_property()const;
        const fw::type*       get_property_type()const;
        bool                  has_node_connected() const;
        ImVec2                position() const;
        Side                  side() const;
        ImRect                get_rect() const;
        Slot&                 slot()const;
        PoolID<Node>          get_node()const;
        PoolID<Node>          adjacent_node() const;
        bool                  is_this() const;
        bool                  allows(SlotFlag) const;
        PoolID<Node>          get_node();
        static SlotView*      get_dragged() { return s_dragged; }
        static SlotView*      get_focused() { return s_dragged; }
        static SlotView*      get_hovered() { return s_hovered; }
        static bool           is_dragging() { return s_dragged; }
        static void           drop_behavior(bool& require_new_node, bool _enable_edition);
        static void           reset_dragged(SlotView * slot = nullptr) { s_dragged = slot; }
        static void           reset_focused(SlotView * slot = nullptr) { s_focused = slot; }
        static void           reset_hovered(SlotView * slot = nullptr) { s_hovered = slot; }
        static void           behavior(SlotView&, bool _readonly);
        static void           draw_slot_circle( SlotView& _view,  ImVec2 _position, float _radius, const ImColor &_color, const ImColor &_borderColor, const ImColor &_hoverColor, bool _readonly );
        static void           draw_slot_rectangle( SlotView& _view, ImVec2 _position, const ImColor &_color, const ImColor &_hoveredColor, bool _readonly);

    private:
        Side                  m_side;
        Slot&                 m_slot;
        ImVec2                m_relative_pos;
        static SlotView*      s_focused;
        static SlotView*      s_dragged;
        static SlotView*      s_hovered;
    };
}