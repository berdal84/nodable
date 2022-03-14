#include <nodable/NodeConnector.h>

#include <nodable/NodeView.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>
#include <nodable/Settings.h>
#include <nodable/AppContext.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "nodable/Event.h"

using namespace Nodable;

const NodeConnector*     NodeConnector::s_dragged   = nullptr;
const NodeConnector*     NodeConnector::s_hovered   = nullptr;
const NodeConnector*     NodeConnector::s_focused   = nullptr;

bool NodeConnector::draw(const NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor, bool _editable)
{
    bool edited = false;
    float rounding = 6.0f;

    auto draw_list = ImGui::GetWindowDrawList();
    auto rect      = _connector->get_rect();
    rect.Translate(ImGuiEx::ToScreenPosOffset());

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
            event.type = EventType::node_connector_disconnected;
            event.node_connectors.src = _connector;
            event.node_connectors.dst = nullptr;
            EventManager::push_event(event);
        }

        ImGui::EndPopup();
    }

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        if ( ImGui::IsMouseDown(0) && !is_dragging() && !NodeView::IsAnyDragged())
        {
            if ( _connector->m_way == Way_Out)
            {
                const auto& successors = _connector->get_node()->successor_slots();
                if (successors.size() < successors.get_limit() ) start_drag(_connector);
            }
            else
            {
                const auto& predecessors = _connector->get_node()->predecessor_slots();
                if (predecessors.size() < predecessors.get_limit() ) start_drag(_connector);
            }
        }

        s_hovered = _connector;
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }

    return edited;
}

ImRect NodeConnector::get_rect() const
{
    Settings* settings = m_context->settings;
    vec2 leftCornerPos = m_way == Way_In ? m_node_view->getRect().GetTL() : m_node_view->getRect().GetBL();

    vec2 size(settings->ui_node_connector_width, settings->ui_node_connector_height);
    ImRect rect(leftCornerPos, leftCornerPos + size);
    rect.Translate(vec2(size.x * float(m_index), -rect.GetSize().y * 0.5f) );
    rect.Expand(vec2(- settings->ui_node_connector_padding, 0.0f));
    return rect;
}

vec2 NodeConnector::get_pos()const
{
    return get_rect().GetCenter() + ImGuiEx::ToScreenPosOffset();
}

bool NodeConnector::share_parent_with(const NodeConnector *other) const
{
    return get_node() == other->get_node();
}

void NodeConnector::dropped(const NodeConnector *_left, const NodeConnector *_right)
{
    Event evt{};
    evt.type = EventType::node_connector_dropped;
    evt.node_connectors.src = _left;
    evt.node_connectors.dst = _right;
    EventManager::push_event(evt);
}

Node* NodeConnector::get_connected_node() const
{
    auto node = get_node();

    if ( m_way == Way_In )
    {
        if (node->predecessor_slots().size() > m_index )
            return node->predecessor_slots()[m_index];
    }
    else if (node->successor_slots().size() > m_index )
    {
        return node->successor_slots()[m_index];
    }
    return nullptr;

}

Node* NodeConnector::get_node()const
{
    return m_node_view->get_owner();
}

