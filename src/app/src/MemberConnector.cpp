#include <nodable/app/MemberConnector.h>

#include <nodable/app/Settings.h>
#include <nodable/core/Node.h>
#include <nodable/core/GraphNode.h>
#include <nodable/app/NodeView.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "nodable/app/Event.h"

using namespace Nodable;

const MemberConnector*   MemberConnector::s_dragged = nullptr;
const MemberConnector*   MemberConnector::s_hovered = nullptr;
const MemberConnector*   MemberConnector::s_focused = nullptr;

vec2 MemberConnector::get_pos()const
{
    vec2 relative_pos_constrained = m_memberView->relative_pos();

    vec2 node_view_size = m_memberView->m_nodeView->get_size();

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
    return vec2(m_memberView->m_nodeView->get_screen_position() + relative_pos_constrained);
}

bool MemberConnector::share_parent_with(const MemberConnector* other) const
{
    return get_member() == other->get_member();
}

void MemberConnector::draw(
        const MemberConnector *_connector,
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
            event.type = EventType::member_connector_disconnected;
            event.member_connectors.src = _connector;
            event.member_connectors.dst = nullptr;
            EventManager::push_event(event);
        }

        ImGui::EndPopup();
    }

    if ( isItemHovered )
    {
        s_hovered = _connector;
        ImGuiEx::BeginTooltip();
        ImGui::Text("%s", _connector->get_member()->get_name().c_str() );
        ImGuiEx::EndTooltip();
    }

    if (isItemHovered)
    {
        if ( ImGui::IsMouseDown(0))
        {
            if ( s_dragged == nullptr && !NodeView::is_any_dragged())
                MemberConnector::start_drag(_connector);
        }
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }
}

void MemberConnector::dropped(const MemberConnector *_left, const MemberConnector *_right)
{
    Event evt{};
    evt.type = EventType::member_connector_dropped;
    evt.member_connectors.src = _left;
    evt.member_connectors.dst = _right;
    EventManager::push_event(evt);
}

bool MemberConnector::has_node_connected() const {
    return m_way == Way_In ? get_member()->get_input() != nullptr : !get_member()->get_outputs().empty();
}

Member* MemberConnector::get_member()const
{
    return m_memberView ?  m_memberView->m_member : nullptr;
}

R::Meta_t_csptr MemberConnector::get_member_type()const
{
    return get_member()->get_meta_type();
}