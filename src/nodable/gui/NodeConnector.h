#pragma once
#include "Connector.h"
#include <imgui/imgui_internal.h>
#include "fw/gui/ImGuiEx.h"
#include "core/Pool.h"

namespace ndbl {

    // forward declarations
    class NodeView;

    /**
     * @brief A NodeConnector represents a physical input or output on a NodeView.
     */
    class NodeConnector: public Connector<NodeConnector>
    {
    public:
        NodeConnector(){}
        NodeConnector(
             NodeView* _nodeView
            , Way _way
            , size_t _index = 0
            , size_t _count = 1);

        ~NodeConnector() = default;
        ID<Node>           get_node()const;
        ID<Node>           get_connected_node() const;
        ImRect             get_rect() const;
        ImVec2             get_pos() const override;
        bool               share_parent_with(const NodeConnector* other) const override;
        static void        draw(NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor, bool _editable);
        static void        dropped(const NodeConnector*, const NodeConnector*);

        size_t    m_index;
        size_t    m_count;
        Way       m_way;
        ID<NodeView> m_node_view;

        static const NodeConnector* s_hovered;
        static const NodeConnector* s_dragged;
        static const NodeConnector* s_focused;
    };
}