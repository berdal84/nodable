#include <nodable/app/PropertyConnector.h>

#include <nodable/app/Settings.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>
#include <nodable/app/NodeView.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "nodable/app/Event.h"

using namespace ndbl;

const PropertyConnector*   PropertyConnector::s_dragged = nullptr;
const PropertyConnector*   PropertyConnector::s_hovered = nullptr;
const PropertyConnector*   PropertyConnector::s_focused = nullptr;

vec2 PropertyConnector::get_pos()const
{
    vec2 relative_pos_constrained = m_propertyView->relative_pos();

    vec2 node_view_size = m_propertyView->m_nodeView->get_size();

    switch (m_display_side)
    {
        case Side::Top:
            relative_pos_constrained.y = -node_view_size.y * 0.5f;
            break;

        case Side::Bottom:
            relative_pos_constrained.y = node_view_size.y * 0.5f;
            break;

        case Side::Left:
            relative_pos_constrained.y = 0;
            relative_pos_constrained.x = -node_view_size.x * 0.5f;
            break;
        case Side::Right:
            relative_pos_constrained.y = 0;
            relative_pos_constrained.x = node_view_size.x * 0.5f;
    }
    return vec2(m_propertyView->m_nodeView->get_screen_position() + relative_pos_constrained);
}

bool PropertyConnector::share_parent_with(const PropertyConnector* other) const
{
    return get_property() == other->get_property();
}

void PropertyConnector::draw(
        const PropertyConnector *_connector,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor,
        bool _editable)
{
    // draw
    //-----
    auto draw_list = ImGui::GetWindowDrawList();
    auto connnectorScreenPos = _connector->get_pos();

    // Unvisible Button on top of the Circle
    vec2 cursorScreenPos = ImGui::GetCursorScreenPos();
    auto invisibleButtonOffsetFactor = 1.2f;
    ImGui::SetCursorScreenPos(connnectorScreenPos - vec2(_radius * invisibleButtonOffsetFactor));
    ImGui::PushID(_connector);
    bool clicked = ImGui::InvisibleButton("###", vec2(_radius * 2.0f * invisibleButtonOffsetFactor, _radius * 2.0f * invisibleButtonOffsetFactor));
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);
    auto isItemHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // Circle
    draw_list->AddCircleFilled(connnectorScreenPos, _radius, isItemHovered ? _hoverColor : _color);
    draw_list->AddCircle(connnectorScreenPos, _radius, _borderColor);

    // behavior
    //--------
    if ( _editable && _connector->has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            Event event{};
            event.type = EventType::property_connector_disconnected;
            event.property_connectors.src = _connector;
            event.property_connectors.dst = nullptr;
            EventManager::push_event(event);
        }

        ImGui::EndPopup();
    }

    if ( isItemHovered )
    {
        s_hovered = _connector;
        if( ImGuiEx::BeginTooltip() )
        {
            ImGui::Text("%s", _connector->get_property()->get_name().c_str() );
            ImGuiEx::EndTooltip();
        }
    }

    if (isItemHovered)
    {
        if ( ImGui::IsMouseDown(0))
        {
            if ( s_dragged == nullptr && !NodeView::is_any_dragged())
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
    Event evt{};
    evt.type = EventType::property_connector_dropped;
    evt.property_connectors.src = _left;
    evt.property_connectors.dst = _right;
    EventManager::push_event(evt);
}

bool PropertyConnector::has_node_connected() const {
    return m_way == Way_In ? get_property()->get_input() != nullptr : !get_property()->get_outputs().empty();
}

Property * PropertyConnector::get_property()const
{
    return m_propertyView ?  m_propertyView->m_property : nullptr;
}

type PropertyConnector::get_property_type()const
{
    return get_property()->get_type();
}