#include "SlotView.h"
#include "Config.h"
#include "Event.h"
#include "Nodable.h"
#include "NodeView.h"
#include "gui.h"

using namespace ndbl;
using namespace tools;

SlotView *SlotView::s_focused = nullptr;
SlotView *SlotView::s_dragged = nullptr;
SlotView *SlotView::s_hovered = nullptr;

SlotView::SlotView(Slot &_slot, Vec2 _alignment )
: m_slot(_slot)
, m_alignment(_alignment)
{
}

void SlotView::draw_slot_circle(
        ImDrawList* _draw_list,
        SlotView& _view,
        Vec2 _position,
        bool _readonly)
{
    float invisible_ratio = g_conf->ui_slot_invisible_ratio;
    float radius          = g_conf->ui_slot_radius;
    Vec4  color           = g_conf->ui_slot_color;
    Vec4  border_color    = g_conf->ui_slot_border_color;
    Vec4  hover_color     = g_conf->ui_slot_hovered_color;

    // draw
    //-----
    Vec2 cursor_pos{_position - Vec2( radius * invisible_ratio )};
    ImGui::SetCursorScreenPos( cursor_pos);

    // draw a larger invisible button on top of the circle to facilitate click/drag
    ImGui::PushID((u8_t)_view.m_slot.id);
    ImGui::InvisibleButton("###", radius * 2.0f * invisible_ratio, radius * 2.0f * invisible_ratio );
    ImGui::PopID();

    bool is_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // draw the circle
    _draw_list->AddCircleFilled( _position, radius, ImColor(is_hovered ? hover_color : color));
    _draw_list->AddCircle( _position, radius, ImColor(border_color) );
    ImGuiEx::DebugCircle( _position, radius, ImColor(border_color));

    behavior(_view, _readonly);
}

void SlotView::draw_slot_rectangle(
    ImDrawList* _draw_list,
    SlotView& _view,
    Rect _rect,
    bool _readonly)
{
    Vec4  color          = g_conf->ui_slot_color;
    Vec4  border_color   = g_conf->ui_slot_border_color;
    float border_radius  = g_conf->ui_slot_border_radius;
    Vec4  hover_color    = g_conf->ui_slot_hovered_color;

    Vec2 rect_size = _rect.size();

    // Return early if rectangle cannot be draw.
    // TODO: Find why size can be zero more (more surprisingly) nan.
    if(rect_size.x == 0.0f || rect_size.y == 0.0f || std::isnan(rect_size.x) || std::isnan(rect_size.y) ) return;

    ImDrawCornerFlags corner_flags = _view.m_slot.has_flags(SlotFlag_ORDER_FIRST) ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    ImGui::SetCursorScreenPos( _rect.tl());
    ImGui::PushID((u8_t)_view.m_slot.id);
    ImGui::InvisibleButton("###", _rect.size());
    ImGui::PopID();

    Vec4 fill_color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? hover_color : color;
    _draw_list->AddRectFilled( _rect.min, _rect.max, ImColor(fill_color), border_radius, corner_flags );
    _draw_list->AddRect( _rect.min, _rect.max, ImColor(border_color), border_radius, corner_flags );
    ImGuiEx::DebugRect( _rect.min, _rect.max, ImColor(255,0, 0, 127), 0.0f );

    behavior(_view, _readonly);
}

void SlotView::behavior(SlotView& _view, bool _readonly)
{
    // Handle disconnect

    if ( !_readonly && _view.has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            auto& event_manager = EventManager::get_instance();
            event_manager.dispatch<Event_SlotDisconnected>({ _view.slot() });
        }

        ImGui::EndPopup();
    }

    // Handle hover state

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        SlotView::reset_hovered(&_view);
        if( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s", SlotView::get_tooltip(_view).c_str() );
            ImGuiEx::EndTooltip();
        }

        if ( ImGui::IsMouseDown(0) && !is_dragging() && !NodeView::is_any_dragged())
        {
            if ( !_view.slot().is_full() )
            {
                reset_dragged( &_view );
            }
        }
    }
    else if (SlotView::get_hovered() == &_view)
    {
        SlotView::reset_hovered();
    }

}

PoolID<Node> SlotView::adjacent_node() const
{
   return m_slot.first_adjacent().node;
}

PoolID<Node> SlotView::get_node()const
{
    return m_slot.node;
}

Vec2 SlotView::position()const
{
    return m_slot.node->get_component<NodeView>()->get_slot_pos( m_slot );
}

Rect SlotView::get_rect()const
{
    return m_slot.node->get_component<NodeView>()->get_slot_rect( *this, 0 );
}

void SlotView::drop_behavior(bool &require_new_node, bool _enable_edition)
{
    if ( s_dragged && ImGui::IsMouseReleased(0) )
    {
        if ( _enable_edition )
        {
            if ( s_hovered )
            {
                auto& event_manager = EventManager::get_instance();
                event_manager.dispatch<Event_SlotDropped>({ s_dragged->m_slot, s_hovered->m_slot});

                reset_hovered();
                reset_dragged();
            }
            else
            {
                require_new_node = true;
            }
        }
        else
        {
            reset_dragged();
        }
    }
}

const type* SlotView::get_property_type()const
{
    Property* property = get_property();
    return property ? property->get_type() : nullptr;
}

Vec2 SlotView::alignment() const
{
    return m_alignment;
}

bool SlotView::is_this() const
{
    return get_property()->is_this();
}

bool SlotView::allows(SlotFlag flags) const
{
    return ( m_slot.flags & flags ) == flags;
}

Slot& SlotView::slot() const
{
    return m_slot;
}

bool SlotView::has_node_connected() const
{
    if ( !m_slot.get_property()->get_type()->is<PoolID<Node>>() )
    {
        return false;
    }

    return m_slot.adjacent_count() != 0;
}

Property* SlotView::get_property() const
{
    return m_slot.get_property();
}

std::string SlotView::get_tooltip( SlotView& _view )
{
    std::string property_name{_view.get_property()->get_name()};

    switch ( _view.slot().static_flags() ) // type and order flags only
    {
        case SlotFlag_INPUT:   return property_name.append(" (in)");
        case SlotFlag_OUTPUT:  return property_name.append(" (out)");
        case SlotFlag_NEXT:    return "next";
        case SlotFlag_PREV:    return "previous";
        case SlotFlag_PARENT:  return "parent";
        case SlotFlag_CHILD:   return "children";
    }
    return property_name;
}
