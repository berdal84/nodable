#pragma once
#include "observe/event.h"
#include "fw/gui/ImGuiEx.h"
#include "core/Connector.h"
#include "core/Way.h"
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
        SlotView(Slot, Side);
        bool                  share_parent_with(SlotView *);
        Property*             get_property()const;
        const fw::type*       get_property_type()const;
        bool                  has_node_connected() const;
        ImVec2                get_pos() const;
        ImRect                get_rect() const;
        ID<Node> node()const;
        ID<Node> adjacent_node() const;
        static SlotView*      get_dragged() { return s_dragged; }
        static SlotView*      get_focused() { return s_dragged; }
        static SlotView*      get_hovered() { return s_hovered; }
        static bool           is_dragging() { return s_dragged; }
        static void           drop_behavior(bool& require_new_node, bool _enable_edition);
        static void           reset_dragged(SlotView * connector = nullptr) { s_dragged = connector; }
        static void           reset_focused(SlotView * connector = nullptr) { s_focused = connector; }
        static void           reset_hovered(SlotView * connector = nullptr) { s_hovered = connector; }
        void draw_connector_circle(
                SlotView *_view,
                float _radius,
                const ImColor &_color,
                const ImColor &_borderColor,
                const ImColor &_hoverColor,
                bool _editable);
        void draw_connector_rectangle(
                SlotView *_view,
                const ImColor &_color,
                const ImColor &_hoveredColor,
                bool _editable);
        bool is_node_connector() const;

        bool allows(Way way) const;

    private:
        Slot                  m_slot;
        Side                  m_display_side;
        static SlotView*      s_focused;
        static SlotView*      s_dragged;
        static SlotView*      s_hovered;
    };
}