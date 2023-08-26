#include "PropertyConnector.h"

#include "core/Node.h"
#include "core/Graph.h"

#include "Config.h"
#include "NodeView.h"
#include "PropertyView.h"
#include "Event.h"
#include "Nodable.h"

using namespace ndbl;

PropertyConnector* PropertyConnector::s_dragged;
PropertyConnector* PropertyConnector::s_hovered;
PropertyConnector* PropertyConnector::s_focused;

PropertyConnector::PropertyConnector()
{}

PropertyConnector::PropertyConnector(
    PropertyView* _property,
    Way _way,
    PropertyConnector::Side _pos
    )
    : m_property_view(_property)
    , m_way(_way)
    , m_display_side(_pos)
{
    FW_ASSERT(_property)
}

ImVec2 PropertyConnector::get_pos() const
{
    ImVec2 node_screen_pos = m_property_view->node_view->get_position(fw::Space_Screen, false);
    ImVec2 node_view_size  = m_property_view->node_view->get_size();
    ImVec2 screen_pos{
            node_screen_pos.x,
            node_screen_pos.y
    };

    switch (m_display_side)
    {
        case Side::Top:
            screen_pos.x  = m_property_view->position.x;
            screen_pos.y -= node_view_size.y * 0.5f;
            break;
        case Side::Bottom:
            screen_pos.x  = m_property_view->position.x;
            screen_pos.y += node_view_size.y * 0.5f;
            break;
        case Side::Left:
            screen_pos.x -= node_view_size.x * 0.5f;
            break;
        case Side::Right:
            screen_pos.x += node_view_size.x * 0.5f;
    }

    fw::ImGuiEx::DebugLine(node_screen_pos, screen_pos, ImColor(0,0,255));

    return screen_pos;
}

bool PropertyConnector::share_parent_with(const PropertyConnector*  other) const
{
    return m_property_view->property() == other->m_property_view->property();
}

void PropertyConnector::draw(
        PropertyConnector* _connector,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor,
        bool _editable)
{
    // draw
    //-----
    auto draw_list = ImGui::GetWindowDrawList();
    auto connector_pos = _connector->get_pos();

    // Unvisible Button on top of the Circle
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    fw::ImGuiEx::DebugCircle(cursor_screen_pos, _radius, ImColor(255,0,0));
    auto invisibleButtonOffsetFactor = 1.2f;
    ImGui::SetCursorScreenPos(connector_pos - ImVec2(_radius * invisibleButtonOffsetFactor));
    ImGui::PushID(_connector);
    bool clicked = ImGui::InvisibleButton("###", ImVec2(_radius * 2.0f * invisibleButtonOffsetFactor, _radius * 2.0f * invisibleButtonOffsetFactor));
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursor_screen_pos);
    auto isItemHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // Circle
    draw_list->AddCircleFilled(connector_pos, _radius, isItemHovered ? _hoverColor : _color);
    draw_list->AddCircle(connector_pos, _radius, _borderColor);

    fw::ImGuiEx::DebugCircle(connector_pos, _radius, _borderColor, ImColor(255, 0, 0, 0));

    // behavior
    //--------
    if ( _editable && _connector->has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            Event event{};
            event.type = EventType_property_connector_disconnected;
            event.connector.src.prop = _connector;
            event.connector.dst.prop = nullptr;
            fw::EventManager::get_instance().push_event((fw::Event&)event);
        }

        ImGui::EndPopup();
    }

    if ( isItemHovered )
    {
        s_hovered = _connector;
        if( fw::ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s", _connector->get_property()->get_name().c_str() );
            fw::ImGuiEx::EndTooltip();
        }
    }

    if (isItemHovered)
    {
        if ( ImGui::IsMouseDown(0) && s_dragged && !NodeView::is_any_dragged())
        {
            PropertyConnector::start_drag(_connector);
        }
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }
}

void PropertyConnector::dropped(const PropertyConnector *_left, const PropertyConnector *_right)
{
    if(!_left || !_right)
    {
        return;
    }
    ConnectorEvent evt{};
    evt.type = EventType_property_connector_dropped;
    evt.src.prop = _left;
    evt.dst.prop = _right;
    fw::EventManager::get_instance().push_event((fw::Event&)evt);
}

bool PropertyConnector::has_node_connected() const
{
    return m_way == Way_In ? !get_property()->get_input() : !get_property()->get_outputs().empty();
}

Property* PropertyConnector::get_property()const
{
    return m_property_view ? m_property_view->property() : nullptr;
}

const fw::type* PropertyConnector::get_property_type()const
{
    Property* property = get_property();
    return property ? property->get_type() : nullptr;
}