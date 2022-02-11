#include <nodable/NodeConnector.h>

#include <nodable/NodeView.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>
#include <nodable/Settings.h>
#include <nodable/AppContext.h>

#include <IconFontCppHeaders/IconsFontAwesome5.h>

using namespace Nodable;

const NodeConnector*     NodeConnector::s_dragged   = nullptr;
const NodeConnector*     NodeConnector::s_hovered   = nullptr;
const NodeConnector*     NodeConnector::s_focused   = nullptr;

bool NodeConnector::Draw(const NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor)
{
    // draw
    float rounding = 6.0f;

    auto draw_list = ImGui::GetWindowDrawList();
    auto rect      = _connector->getRect();
    rect.Translate(ImGuiEx::ToScreenPosOffset());

    ImDrawCornerFlags cornerFlags = _connector->m_way == Way_Out ? ImDrawCornerFlags_Bot : ImDrawCornerFlags_Top;

    auto cursorScreenPos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(rect.GetTL());
    ImGui::PushID(_connector);
    bool clicked = ImGui::InvisibleButton("###", rect.GetSize());
    ImGui::PopID();
    ImGui::SetCursorScreenPos(cursorScreenPos);

    ImColor color = ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) ? _hoveredColor : _color;
    draw_list->AddRectFilled(rect.Min, rect.Max, color, rounding, cornerFlags );
    draw_list->AddRect(rect.Min, rect.Max, ImColor(50,50, 50), rounding, cornerFlags );

    // behavior
    auto connectedNode = _connector->getConnectedNode();
    if ( connectedNode && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            auto node   = _connector->getNode();
            auto graph  = node->get_parent_graph();

            if ( _connector->m_way == Way_In )
                graph->disconnect(node, connectedNode, Relation_t::IS_SUCCESSOR_OF);
            else
                graph->disconnect(connectedNode, node, Relation_t::IS_SUCCESSOR_OF);
        }

        ImGui::EndPopup();
    }

    if ( ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) )
    {
        if (ImGui::IsMouseDown(0) && !IsDragging() && !NodeView::IsAnyDragged())
        {
            if ( _connector->m_way == Way_Out)
            {
                if (_connector->getNode()->successor_slots().size() <
                        _connector->getNode()->successor_slots().get_limit() )
                    StartDrag(_connector);
            }
            else
            {
                if (_connector->getNode()->predecessor_slots().size() <
                        _connector->getNode()->predecessor_slots().get_limit() )
                    StartDrag(_connector);
            }
        }

        s_hovered = _connector;
    }
    else if ( s_hovered == _connector )
    {
        s_hovered = nullptr;
    }

    return clicked;
}

ImRect NodeConnector::getRect() const
{
    Settings* settings = m_context->settings;
    vec2 leftCornerPos = m_way == Way_In ? m_nodeView->getRect().GetTL() : m_nodeView->getRect().GetBL();

    vec2 size(settings->ui_node_connector_width, settings->ui_node_connector_height);
    ImRect rect(leftCornerPos, leftCornerPos + size);
    rect.Translate(vec2(size.x * float(m_index), -rect.GetSize().y * 0.5f) );
    rect.Expand(vec2(- settings->ui_node_connector_padding, 0.0f));
    return rect;
}

vec2 NodeConnector::getPos()const
{
    return getRect().GetCenter() + ImGuiEx::ToScreenPosOffset();
}

bool NodeConnector::connect(const NodeConnector* other) const
{
    auto graph = getNode()->get_parent_graph();
    // TODO: handle incompatibility
    graph->connect(other->getNode(), getNode() , Relation_t::IS_SUCCESSOR_OF );

    return true;
}

void NodeConnector::DropBehavior(bool &needsANewNode)
{
    if (s_dragged && ImGui::IsMouseReleased(0))
    {
        if ( s_hovered )
        {
            NodeConnector::Connect(s_dragged, s_hovered);
            s_dragged = s_hovered = nullptr;
        } else {
            needsANewNode = true;
        }
    }
}

bool NodeConnector::hasSameParentWith(const NodeConnector *other) const
{
    return getNode() == other->getNode();
}

bool NodeConnector::Connect(const NodeConnector *_left, const NodeConnector *_right)
{
    if ( _left->hasSameParentWith(_right) )
    {
        LOG_WARNING("NodeConnector", "Unable to connect these two Connectors from the same Node.\n")
        return false;
    }

    if( _left->m_way == _right->m_way )
    {
        LOG_WARNING("NodeConnector", "Unable to connect these two Node Connectors (must have different ways).\n")
        return false;
    }

    if ( _left->m_way == Way_Out )
        return _left->connect(_right);
    return _right->connect(_left);
}

Node* NodeConnector::getConnectedNode() const
{
    auto node = getNode();

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

Node* NodeConnector::getNode()const
{
    return m_nodeView->get_owner();
}

