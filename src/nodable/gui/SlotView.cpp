#include "SlotView.h"
#include "NodeView.h"
#include "Event.h"
#include "Nodable.h"

using namespace ndbl;

SlotView *SlotView::s_focused = nullptr;
SlotView *SlotView::s_dragged = nullptr;
SlotView *SlotView::s_hovered = nullptr;

SlotView::SlotView(Slot _slot, Side _side)
    : m_slot(_slot)
      , m_display_side( _side )
{
}

void SlotView::draw_connector_circle(
        SlotView* _view,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor,
        bool _editable)
{
    // draw
    //-----
    auto draw_list = ImGui::GetWindowDrawList();
    auto view_pos = _view->get_pos();

    // Unvisible Button on top of the Circle
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    fw::ImGuiEx::DebugCircle(cursor_screen_pos, _radius, ImColor(255,0,0));
    auto invisibleButtonOffsetFactor = 1.2f;
    ImGui::SetCursorScreenPos(view_pos - ImVec2(_radius * invisibleButtonOffsetFactor));
    ImGui::PushID(_view);
    bool clicked = ImGui::InvisibleButton("###", ImVec2(_radius * 2.0f * invisibleButtonOffsetFactor, _radius * 2.0f * invisibleButtonOffsetFactor));
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursor_screen_pos);
    auto is_hovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // Circle
    draw_list->AddCircleFilled(view_pos, _radius, is_hovered ? _hoverColor : _color);
    draw_list->AddCircle(view_pos, _radius, _borderColor);

    fw::ImGuiEx::DebugCircle(view_pos, _radius, _borderColor, ImColor(255, 0, 0, 0));

    // behavior
    //--------
    if ( _editable && _view->has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            Event event{};
            event.type = EventType_connector_disconnected;
            event.connector.first = _view;
            fw::EventManager::get_instance().push_event((fw::Event&)event);
        }

        ImGui::EndPopup();
    }

    if (is_hovered)
    {
        SlotView::reset_hovered(_view);
        if( fw::ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s", _view->get_property()->get_name().c_str() );
            fw::ImGuiEx::EndTooltip();
        }

        if ( ImGui::IsMouseDown(0) && SlotView::is_dragging() && !NodeView::is_any_dragged())
        {
            reset_dragged(_view);
        }
    }
    else if (SlotView::get_hovered() == _view)
    {
        SlotView::reset_hovered();
    }
}

void SlotView::draw_connector_rectangle(
        SlotView*_view,
        const ImColor &_color,
        const ImColor &_hoveredColor,
        bool _editable)
{
    constexpr float rounding = 6.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRect      rect      = _view->get_rect();
    ImVec2      rect_size = rect.GetSize();

    // Return early if rectangle cannot be draw.
    // TODO: Find why size can be zero more (more surprisingly) nan.
    if(rect_size.x == 0.0f || rect_size.y == 0.0f || std::isnan(rect_size.x) || std::isnan(rect_size.y) ) return;

    ImDrawCornerFlags cornerFlags = _view->m_slot.connector.way == Way::Out ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    auto cursorScreenPos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(rect.GetTL());
    ImGui::PushID(_view);
    ImGui::InvisibleButton("###", rect.GetSize());
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);

    ImColor color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? _hoveredColor : _color;
    draw_list->AddRectFilled(rect.Min, rect.Max, color, rounding, cornerFlags );
    draw_list->AddRect(rect.Min, rect.Max, ImColor(50,50, 50), rounding, cornerFlags );
    fw::ImGuiEx::DebugRect(rect.Min, rect.Max, ImColor(255,0, 0, 127), 0.0f );

    // behavior
    if (_editable && (bool) _view->node() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            SlotEvent event{};
            event.type = EventType_node_connector_disconnected;
            event.first = _view;
            event.dst = nullptr;
            fw::EventManager::get_instance().push_event((fw::Event&)event);
        }

        ImGui::EndPopup();
    }

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        if ( ImGui::IsMouseDown(0) && !is_dragging() && !NodeView::is_any_dragged())
        {

            if (_view->m_slot.connector.way == Way::Out)
            {
                if ( _view->node()->allows_more(Relation::NEXT_PREVIOUS) )
                {
                    reset_dragged( _view );
                }
            }
            else if (_view->node()->allows_more(Relation::NEXT_PREVIOUS) )
            {
                reset_dragged(_view);
            }
        }
        s_hovered = _view;
    }
    else if ( s_hovered == _view)
    {
        s_hovered = nullptr;
    }
}

ImRect SlotView::get_rect() const
{
    Config&      config    = Nodable::get_instance().config;
    ID<NodeView> node_view = node()->get_component<NodeView>();

    // pick a corner
    ImVec2   left_corner = m_slot.connector.way == Way::In ?
                                         node_view->get_screen_rect().GetTL() : node_view->get_screen_rect().GetBL();

    // compute connector size
    ImVec2 size(
            std::min(config.ui_node_connector_width,  node_view->get_size().x),
            std::min(config.ui_node_connector_height, node_view->get_size().y));
    ImRect rect(left_corner, left_corner + size);
    rect.Translate(ImVec2(size.x * float(m_slot.index), -rect.GetSize().y * 0.5f) );
    rect.Expand(ImVec2(- config.ui_node_connector_padding, 0.0f));

    FW_EXPECT(false, "TODO: use m_display_side instead of way");
    FW_EXPECT(false, "TODO: generate a relative rectangle (relative to node bbox)");

    return rect;
}


ID<Node> SlotView::adjacent_node() const
{
    m_slot.first_adjacent_connector().node;
}

ID<Node> SlotView::node()const
{
    return m_slot.connector.node;
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
                evt.type = EventType_connector_dropped;
                evt.first = s_dragged;
                evt.dst  = s_hovered;
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

ImVec2 SlotView::get_pos() const
{
    ImVec2 screen_pos;

    switch (m_display_side)
    {
        case Side::Top:
            screen_pos.x = 0;
            screen_pos.y = -.5f;
            break;
        case Side::Bottom:
            FW_EXPECT(false, "TODO: Assign property position x (relative to node's bbox)");
            screen_pos.x = 0.0f;
            screen_pos.y = 0.5f;
            break;
        case Side::Left:
            screen_pos.x = -0.5f;
            screen_pos.y = 0.f;
            break;
        case Side::Right:
            screen_pos.x = 0.5f;
            screen_pos.y = 0.f;

    }

    FW_EXPECT(false, "TODO: This result should be interpreted as relative to node's bbox.");
    return screen_pos;
}

bool SlotView::is_node_connector() const
{
    return get_property()->is_referencing<Node>();
}

bool SlotView::allows(Way way) const
{
    return m_slot.connector.allows(way);
}
