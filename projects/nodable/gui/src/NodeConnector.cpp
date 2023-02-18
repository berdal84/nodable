#include <ndbl/gui/NodeConnector.h>

#include "ndbl/gui/App.h"
#include <ndbl/core/GraphNode.h>
#include <ndbl/core/Node.h>
#include <ndbl/gui/Event.h>
#include <ndbl/gui/NodeView.h>
#include <ndbl/gui/Settings.h>

using namespace ndbl;

const NodeConnector*     NodeConnector::s_dragged   = nullptr;
const NodeConnector*     NodeConnector::s_hovered   = nullptr;
const NodeConnector*     NodeConnector::s_focused   = nullptr;

void NodeConnector::draw(const NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor, bool _editable)
{
    float rounding = 6.0f;

    auto draw_list = ImGui::GetWindowDrawList();
    auto rect      = _connector->get_rect();
    rect.Translate(fw::ImGuiEx::ToScreenPosOffset());

    ImDrawCornerFlags cornerFlags = _connector->m_way == Way_Out ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    auto cursorScreenPos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(rect.GetTL());
    ImGui::PushID(_connector);
    ImGui::InvisibleButton("###", rect.GetSize());
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);

    ImColor color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? _hoveredColor : _color;
    draw_list->AddRectFilled(rect.Min, rect.Max, color, rounding, cornerFlags );
    draw_list->AddRect(rect.Min, rect.Max, ImColor(50,50, 50), rounding, cornerFlags );

    // behavior
    auto connectedNode = _connector->get_connected_node();
    if ( _editable && connectedNode && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            Event event{};
            event.type = EventType_node_connector_disconnected;
            event.node_connectors.src = _connector;
            event.node_connectors.dst = nullptr;
            fw::EventManager::get_instance().push_event((fw::Event&)event);
        }

        ImGui::EndPopup();
    }

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        if ( ImGui::IsMouseDown(0) && !is_dragging() && !NodeView::is_any_dragged())
        {
            if ( _connector->m_way == Way_Out)
            {
                const auto& successors = _connector->get_node()->successors();
                if (successors.size() < successors.get_limit() ) start_drag(_connector);
            }
            else
            {
                const auto& predecessors = _connector->get_node()->predecessors();
                if (predecessors.size() < predecessors.get_limit() ) start_drag(_connector);
            }
        }

        s_hovered = _connector;
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }
}

ImRect NodeConnector::get_rect() const
{
    App& app = App::get_instance();
    ImVec2 leftCornerPos = m_way == Way_In ? m_node_view.get_rect().GetTL() : m_node_view.get_rect().GetBL();

    ImVec2 size(app.settings.ui_node_connector_width, app.settings.ui_node_connector_height);
    ImRect rect(leftCornerPos, leftCornerPos + size);
    rect.Translate(ImVec2(size.x * float(m_index), -rect.GetSize().y * 0.5f) );
    rect.Expand(ImVec2(- app.settings.ui_node_connector_padding, 0.0f));
    return rect;
}

ImVec2 NodeConnector::get_pos()const
{
    return get_rect().GetCenter() + fw::ImGuiEx::ToScreenPosOffset();
}

bool NodeConnector::share_parent_with(const NodeConnector *other) const
{
    return get_node() == other->get_node();
}

void NodeConnector::dropped(const NodeConnector *_left, const NodeConnector *_right)
{
    NodeConnectorEvent evt{};
    evt.type = EventType_node_connector_dropped;
    evt.src = _left;
    evt.dst = _right;
    fw::EventManager::get_instance().push_event((fw::Event&)evt);
}

Node* NodeConnector::get_connected_node() const
{
    auto node = get_node();

    if ( m_way == Way_In )
    {
        if (node->predecessors().size() > m_index )
        {
            return node->predecessors()[m_index];
        }
    }
    else if (node->successors().size() > m_index )
    {
        return node->successors()[m_index];
    }
    return nullptr;

}

Node* NodeConnector::get_node()const
{
    return m_node_view.get_owner();
}

