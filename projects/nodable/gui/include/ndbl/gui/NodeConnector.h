#pragma once
#include <ndbl/gui/Connector.h>
#include <imgui/imgui_internal.h>

namespace ndbl {

    // forward declarations
    class NodeView;

    /**
     * @brief A NodeConnector represents a physical input or output on a NodeView.
     */
    class NodeConnector: public Connector<NodeConnector>
    {
    public:
        NodeConnector(
              NodeView& _nodeView
            , Way _way
            , size_t _index = 0
            , size_t _count = 1)
        : m_node_view(_nodeView)
        , m_way(_way)
        , m_index(_index)
        , m_count(_count)
        {};

        ~NodeConnector() = default;
        Node*              get_node()const;
        Node*              get_connected_node() const;
        ImRect             get_rect()const;
        ImVec2             get_pos()const override;
        bool               share_parent_with(const NodeConnector *other) const override;
        static void        draw(const NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor, bool _editable);
        static void        dropped(const NodeConnector *_left, const NodeConnector *_right);

        size_t    m_index;
        size_t    m_count;
        NodeView& m_node_view;
        Way       m_way;

        static const NodeConnector* s_hovered;
        static const NodeConnector* s_dragged;
        static const NodeConnector* s_focused;
    };
}