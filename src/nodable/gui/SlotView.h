#pragma once
#include "Config.h"
#include "core/Slot.h"
#include "core/reflection/type.h"
#include "fw/gui/ImGuiEx.h"
#include "observe/event.h"
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
        SlotView( Slot& _slot, ImVec2 _alignment );

        Property*             get_property()const;
        const fw::type*       get_property_type()const;
        bool                  has_node_connected() const;
        ImVec2                alignment() const;
        Slot&                 slot()const;
        PoolID<Node>          get_node()const;
        ImVec2                position()const;
        ImRect                rect(Config& config)const;
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
        static void           draw_slot_circle( ImDrawList* _draw_list, SlotView& _view,  ImVec2 _position, float _radius, const ImColor &_color, const ImColor &_border_color, const ImColor &_hover_color, bool _readonly );
        static void           draw_slot_rectangle(ImDrawList* _draw_list, SlotView& _view, ImRect _rect, const ImColor& _color, const ImColor& _border_color, const ImColor& _hover_color, float _border_radius, bool _readonly);

    private:
        Slot&                 m_slot;
        ImVec2                m_alignment;
        static SlotView*      s_focused;
        static SlotView*      s_dragged;
        static SlotView*      s_hovered;
        static std::string get_tooltip( SlotView &view );
    };
}