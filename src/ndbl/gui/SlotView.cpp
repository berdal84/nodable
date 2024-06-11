#include "SlotView.h"
#include "Config.h"
#include "Event.h"
#include "NodeView.h"
using namespace ndbl;
using namespace tools;

SlotView::SlotView(Slot &_slot, Vec2 _alignment )
: m_slot(_slot)
, m_alignment(_alignment)
{
}

void SlotView::draw_slot_circle(
        ImDrawList* _draw_list,
        SlotView& _view,
        const Vec2& _position)
{
    Config* cfg = get_config();
    float invisible_ratio = cfg->ui_slot_invisible_ratio;
    float radius          = cfg->ui_slot_radius;
    Vec4  color           = cfg->ui_slot_color;
    Vec4  border_color    = cfg->ui_slot_border_color;
    Vec4  hover_color     = cfg->ui_slot_hovered_color;

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
}

void SlotView::draw_slot_rectangle(
    ImDrawList* _draw_list,
    SlotView& _view,
    const Rect& _rect
    )
{
    Config* cfg = get_config();
    Vec4  color          = cfg->ui_slot_color;
    Vec4  border_color   = cfg->ui_slot_border_color;
    float border_radius  = cfg->ui_slot_border_radius;
    Vec4  hover_color    = cfg->ui_slot_hovered_color;

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

std::string SlotView::get_tooltip() const
{
    std::string property_name{get_property()->get_name()};

    switch ( m_slot.static_flags() ) // type and order flags only
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
