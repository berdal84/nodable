#include "NodeConnector.h"

#include "core/Graph.h"
#include "core/Node.h"

#include "Config.h"
#include "Event.h"
#include "NodeView.h"
#include "Nodable.h"
#include "core/Pool.h"

using namespace ndbl;

const NodeConnector* NodeConnector::s_dragged;
const NodeConnector* NodeConnector::s_hovered;
const NodeConnector* NodeConnector::s_focused;

NodeConnector::NodeConnector(
     NodeView* _nodeView
    , Way _way
    , size_t _index
    , size_t _count
    )
    : m_node_view(_nodeView->id())
    , m_way(_way)
    , m_index(_index)
    , m_count(_count)
{};

void NodeConnector::draw(NodeConnector* _connector, const ImColor &_color, const ImColor &_hoveredColor, bool _editable)
{
    constexpr float rounding = 6.0f;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImRect      rect      = _connector->get_rect();
    ImVec2      rect_size = rect.GetSize();

    // Return early if rectangle cannot be draw.
    // TODO: Find why size can be zero more (more surprisingly) nan.
    if(rect_size.x == 0.0f || rect_size.y == 0.0f || std::isnan(rect_size.x) || std::isnan(rect_size.y) ) return;

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
    fw::ImGuiEx::DebugRect(rect.Min, rect.Max, ImColor(255,0, 0, 127), 0.0f );

    // behavior
    if (_editable && (bool)_connector->get_connected_node() && ImGui::BeginPopupContextItem() )
    {
        if ( ImGui::MenuItem(ICON_FA_TRASH " Disconnect"))
        {
            ConnectorEvent event{};
            event.type = EventType_node_connector_disconnected;
            event.src.node = _connector;
            event.dst.node = nullptr;
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
                Node* node = _connector->get_node().get();
                if (node->successors.size() < node->successors.get_limit() )
                {
                    start_drag(_connector);
                }
            }
            else
            {
                if ( _connector->get_node()->predecessors.empty() )
                {
                    start_drag(_connector);
                }
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
    Config& config         = Nodable::get_instance().config;
    NodeView* node_view    = m_node_view.get();

    // pick a corner
    ImVec2   left_corner = m_way == Way_In ?
                           node_view->get_screen_rect().GetTL() : node_view->get_screen_rect().GetBL();

    // compute connector size
    ImVec2 size(
            std::min(config.ui_node_connector_width,  node_view->get_size().x),
            std::min(config.ui_node_connector_height, node_view->get_size().y));
    ImRect rect(left_corner, left_corner + size);
    rect.Translate(ImVec2(size.x * float(m_index), -rect.GetSize().y * 0.5f) );
    rect.Expand(ImVec2(- config.ui_node_connector_padding, 0.0f));

    return rect;
}

ImVec2 NodeConnector::get_pos() const
{
    return get_rect().GetCenter();
}

bool NodeConnector::share_parent_with(const NodeConnector* other) const
{
    return get_node() == other->get_node();
}

void NodeConnector::dropped(const NodeConnector* _left, const NodeConnector* _right)
{
    ConnectorEvent evt{};
    evt.type = EventType_node_connector_dropped;
    evt.src.node = _left;
    evt.dst.node = _right;
    fw::EventManager::get_instance().push_event((fw::Event&)evt);
}

ID<Node> NodeConnector::get_connected_node() const
{
    Node* node = get_node().get();

    if ( m_way == Way_In )
    {
        if (node->predecessors.size() > m_index )
        {
            return node->predecessors[m_index];
        }
    }
    else if (node->successors.size() > m_index )
    {
        return node->successors[m_index];
    }
    return {};
}

ID<Node> NodeConnector::get_node()const
{
    return m_node_view->get_owner();
}

