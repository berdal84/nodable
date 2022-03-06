#include <nodable/MemberConnector.h>

#include <nodable/Settings.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>
#include <nodable/NodeView.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

const MemberConnector*   MemberConnector::s_dragged = nullptr;
const MemberConnector*   MemberConnector::s_hovered = nullptr;
const MemberConnector*   MemberConnector::s_focused = nullptr;

vec2 MemberConnector::get_pos()const
{
    vec2 relative_pos_constrained = m_memberView->relative_pos();

    vec2 node_view_size = m_memberView->m_nodeView->getSize();

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
    return vec2( m_memberView->m_nodeView->getScreenPos() + relative_pos_constrained);
}

bool MemberConnector::share_parent_with(const MemberConnector* other) const
{
    return get_member() == other->get_member();
}

void MemberConnector::drop_behavior(bool &require_new_node, bool& has_made_connection)
{
    if (s_dragged && ImGui::IsMouseReleased(0))
    {
        if ( s_hovered )
        {
            MemberConnector::connect(s_dragged, s_hovered);
            s_dragged = s_hovered = nullptr;
            has_made_connection = true;
        } else {
            require_new_node = true;
        }
    }
}

bool MemberConnector::draw(
        const MemberConnector *_connector,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor)
{
    // draw
    //-----
    bool edited = false;
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
    if (_connector->has_node_connected() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            auto member = _connector->get_member();
            auto graph  = member->get_owner()->get_parent_graph();
            graph->disconnect( member, _connector->m_way );
            edited = true;
        }

        ImGui::EndPopup();
    }

    if ( isItemHovered )
    {
        s_hovered = _connector;
        ImGui::BeginTooltip();
        ImGui::Text("%s", _connector->get_member()->get_name().c_str() );
        ImGui::EndTooltip();
    }

    if (isItemHovered)
    {
        if (ImGui::IsMouseDown(0))
        {
            if ( s_dragged == nullptr && !NodeView::IsAnyDragged())
                MemberConnector::start_drag(_connector);
        }
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }

    return edited;
}

bool MemberConnector::connect(const MemberConnector *_left, const MemberConnector *_right)
{
    if (_left->share_parent_with(_right) )
    {
        LOG_WARNING( "MemberConnector", "Unable to connect two connectors from the same Member.\n" )
        return false;
    }

    if (_left->m_display_side == _right->m_display_side)
    {
        LOG_WARNING( "MemberConnector", "Unable to connect two connectors with the same nature (in and in, out and out)\n" )
        return false;
    }

    GraphNode* graph = _left->get_member()->get_owner()->get_parent_graph();
    if (s_dragged->m_way == Way_Out )
        graph->connect( _left->get_member(), _right->get_member() );
    else
        graph->connect( _right->get_member(), _left->get_member() );

    return true;
}

bool MemberConnector::has_node_connected() const {
    return m_way == Way_In ? get_member()->get_input() != nullptr : !get_member()->get_outputs().empty();
}

Member* MemberConnector::get_member()const
{
    return m_memberView ?  m_memberView->m_member : nullptr;
}

R::Type_ptr MemberConnector::get_member_type()const
{
    return get_member()->get_type();
}