#include "SlotView.h"
#include "NodeView.h"
#include "Event.h"
#include "Nodable.h"

using namespace ndbl;

SlotView *SlotView::s_focused = nullptr;
SlotView *SlotView::s_dragged = nullptr;
SlotView *SlotView::s_hovered = nullptr;

SlotView::SlotView(Slot& _slot, Side _side)
: m_slot(_slot)
, m_side(_side )
, m_relative_pos(0.f, 0.f)
{
    switch ( m_side )
    {
        case Side::Top:    m_relative_pos.y = -0.5f; break;
        case Side::Bottom: m_relative_pos.y =  0.5f; break;
        case Side::Left:   m_relative_pos.x = -0.5f; break;
        case Side::Right:  m_relative_pos.x =  0.5f;
    }
}

void SlotView::draw_slot_circle(
        SlotView &_view,
        ImVec2 _position,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor,
        bool _readonly)
{
    // draw
    //-----
    auto draw_list = ImGui::GetWindowDrawList();

    // Unvisible Button on top of the Circle
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    fw::ImGuiEx::DebugCircle(cursor_screen_pos, _radius, ImColor(255,0,0));
    auto invisibleButtonOffsetFactor = 1.2f;
    ImGui::SetCursorScreenPos( _position - ImVec2(_radius * invisibleButtonOffsetFactor));
    ImGui::PushID(_view.m_slot.id);
    bool clicked = ImGui::InvisibleButton("###", ImVec2(_radius * 2.0f * invisibleButtonOffsetFactor, _radius * 2.0f * invisibleButtonOffsetFactor));
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursor_screen_pos);
    auto is_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // Circle
    draw_list->AddCircleFilled( _position, _radius, is_hovered ? _hoverColor : _color);
    draw_list->AddCircle( _position, _radius, _borderColor);

    fw::ImGuiEx::DebugCircle( _position, _radius, _borderColor, ImColor(255, 0, 0, 0));

    behavior(_view, _readonly);
}

void SlotView::draw_slot_rectangle(
        SlotView& _view,
        ImVec2 _position,
        const ImColor& _color,
        const ImColor& _hoveredColor,
        bool _readonly)
{
    constexpr float rounding = 6.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRect      rect      = _view.get_rect();
    ImVec2      rect_size = rect.GetSize();

    // Return early if rectangle cannot be draw.
    // TODO: Find why size can be zero more (more surprisingly) nan.
    if(rect_size.x == 0.0f || rect_size.y == 0.0f || std::isnan(rect_size.x) || std::isnan(rect_size.y) ) return;

    ImDrawCornerFlags cornerFlags = _view.m_slot.flags & SlotFlag_ACCEPTS_DEPENDENTS ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    auto cursorScreenPos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(rect.GetTL());
    ImGui::PushID(_view.m_slot.id);
    ImGui::InvisibleButton("###", rect.GetSize());
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);

    ImColor color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? _hoveredColor : _color;
    draw_list->AddRectFilled(rect.Min, rect.Max, color, rounding, cornerFlags );
    draw_list->AddRect(rect.Min, rect.Max, ImColor(50,50, 50), rounding, cornerFlags );
    fw::ImGuiEx::DebugRect(rect.Min, rect.Max, ImColor(255,0, 0, 127), 0.0f );

    behavior(_view, _readonly);
    FW_EXPECT(false, "Behavior should be extracted and unified");
}

void SlotView::behavior(SlotView& _view, bool _readonly)
{
    // Handle disconnect

    if ( !_readonly && _view.has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            Event event{};
            event.type = EventType_slot_disconnected;
            event.slot.first = _view.m_slot;
            fw::EventManager::get_instance().push_event((fw::Event&)event);
        }

        ImGui::EndPopup();
    }

    // Handle hover state

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        SlotView::reset_hovered(&_view);
        if( fw::ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s", _view.get_property()->get_name().c_str() );
            fw::ImGuiEx::EndTooltip();
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

ImRect SlotView::get_rect() const
{
    Config&   config    = Nodable::get_instance().config;
    NodeView* node_view = get_node()->get_component<NodeView>().get();

    // pick a corner
    FW_EXPECT(false, "TODO: generate a relative rectangle (relative to node bbox)");
    ImRect node_view_rect = node_view->get_screen_rect();
    ImVec2 left_corner    = m_side == Side::Top ? node_view_rect.GetTL() : node_view_rect.GetBL();

    // compute slot size
    FW_EXPECT(false, "TODO: This code is for this_slots only, handle it in the constructor");
    ImVec2 size(
            std::min(config.ui_node_slot_width,  node_view->get_size().x),
            std::min(config.ui_node_slot_height, node_view->get_size().y));
    ImRect rect(left_corner, left_corner + size);
    rect.Translate(ImVec2(size.x * float(m_slot.id), -rect.GetSize().y * 0.5f) );
    rect.Expand(ImVec2(- config.ui_node_slot_padding, 0.0f));

    return rect;
}


PoolID<Node> SlotView::adjacent_node() const
{
   return m_slot.first_adjacent().node;
}

PoolID<Node> SlotView::get_node()const
{
    return m_slot.node;
}

PoolID<Node> SlotView::get_node()
{
    return m_slot.node;
}

void SlotView::drop_behavior(bool &require_new_node, bool _enable_edition)
{
    if ( s_dragged && ImGui::IsMouseReleased(0) )
    {
        if ( _enable_edition )
        {
            if ( s_hovered )
            {
                SlotEvent evt{};
                evt.type = EventType_slot_dropped;
                evt.first  = s_dragged->m_slot;
                evt.second = s_hovered->m_slot;
                fw::EventManager::get_instance().push_event((fw::Event&)evt);

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

const fw::type* SlotView::get_property_type()const
{
    Property* property = get_property();
    return property ? property->get_type() : nullptr;
}

ImVec2 SlotView::position() const
{
    return m_relative_pos;
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

Side SlotView::side() const
{
    return m_side;
}
