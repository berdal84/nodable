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

ImVec2 MemberConnector::getPos()const
{
    ImVec2 relative_pos_constrained = m_memberView->relative_pos();

    ImVec2 node_view_size = m_memberView->m_nodeView->getSize();

    if (m_display_side == Side::Top )
    {
        relative_pos_constrained.y = - node_view_size.y * 0.5f;
    }
    else if (m_display_side == Side::Bottom )
    {
        relative_pos_constrained.y = node_view_size.y * 0.5f;
    }
    else if (m_display_side == Side::Left )
    {
        relative_pos_constrained.y = 0;
        relative_pos_constrained.x = -node_view_size.x * 0.5f;
    }
    else if (m_display_side == Side::Right )
    {
        relative_pos_constrained.y = 0;
        relative_pos_constrained.x = node_view_size.x * 0.5f;
    }

    return ImVec2( m_memberView->m_nodeView->getScreenPos() + relative_pos_constrained);
}

bool MemberConnector::hasSameParentWith(const MemberConnector* other) const
{
    return getMember() == other->getMember();
}

bool MemberConnector::connect(const MemberConnector *other) const
{
    auto graph = getMember()->get_owner()->get_parent_graph();
    // TODO: handle incompatibility
    graph->connect(getMember(), other->getMember());
    return true;
}

void MemberConnector::DropBehavior(bool &needsANewNode)
{
    if (s_dragged && ImGui::IsMouseReleased(0))
    {
        if ( s_hovered )
        {
            MemberConnector::Connect(s_dragged, s_hovered);
            s_dragged = s_hovered = nullptr;
        } else {
            needsANewNode = true;
        }
    }
}

void MemberConnector::Draw(
        const MemberConnector *_connector,
        float _radius,
        const ImColor &_color,
        const ImColor &_borderColor,
        const ImColor &_hoverColor)
{
    // draw
    //-----

    auto draw_list = ImGui::GetWindowDrawList();
    auto connnectorScreenPos = _connector->getPos();

    // Unvisible Button on top of the Circle
    ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
    auto invisibleButtonOffsetFactor = 1.2f;
    ImGui::SetCursorScreenPos(connnectorScreenPos - ImVec2(_radius * invisibleButtonOffsetFactor));
    ImGui::PushID(_connector);
    bool clicked = ImGui::InvisibleButton("###", ImVec2(_radius * 2.0f * invisibleButtonOffsetFactor, _radius * 2.0f * invisibleButtonOffsetFactor));
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);
    auto isItemHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly);

    // Circle
    draw_list->AddCircleFilled(connnectorScreenPos, _radius, isItemHovered ? _hoverColor : _color);
    draw_list->AddCircle(connnectorScreenPos, _radius, _borderColor);

    // behavior
    //--------
    if ( _connector->hasConnectedNode() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            auto member = _connector->getMember();
            auto graph  = member->get_owner()->get_parent_graph();
            graph->disconnect( member, _connector->m_way );
        }

        ImGui::EndPopup();
    }

    if ( isItemHovered )
    {
        s_hovered = _connector;
        ImGui::BeginTooltip();
        ImGui::Text("%s", _connector->getMember()->get_name().c_str() );
        ImGui::EndTooltip();
    }

    if (isItemHovered)
    {
        if (ImGui::IsMouseDown(0))
        {
            if ( s_dragged == nullptr && !NodeView::IsAnyDragged())
                MemberConnector::StartDrag(_connector);
        }
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }
}

bool MemberConnector::Connect(const MemberConnector *_left, const MemberConnector *_right)
{
    if ( _left->hasSameParentWith(_right) )
    {
        LOG_WARNING( "MemberConnector", "Unable to connect two connectors from the same Member.\n" )
        return false;
    }

    if (_left->m_display_side == _right->m_display_side)
    {
        LOG_WARNING( "MemberConnector", "Unable to connect two connectors with the same nature (in and in, out and out)\n" )
        return false;
    }

    if (s_dragged->m_way == Way_Out )
        return s_dragged->connect(s_hovered);
    return s_hovered->connect(s_dragged);
}

bool MemberConnector::hasConnectedNode() const {
    return m_way == Way_In ? getMember()->get_input() != nullptr : !getMember()->get_outputs().empty();
}

Member* MemberConnector::getMember()const
{
    return m_memberView->m_member;
}
