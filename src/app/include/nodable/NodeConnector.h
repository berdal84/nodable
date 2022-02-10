#pragma once
#include <nodable/Connector.h>
#include <imgui/imgui_internal.h>

namespace Nodable {

    // forward declarations
    class NodeView;
    class AppContext;

    /**
     * @brief A NodeConnector represents a physical input or output on a NodeView.
     */
    class NodeConnector: public Connector<NodeConnector>
    {
    public:
        NodeConnector(
              const AppContext* _ctx
            , NodeView* _nodeView
            , Way _way
            , size_t _index = 0
            , size_t _count = 1)
        : m_context( _ctx )
        , m_nodeView(_nodeView)
        , m_way(_way)
        , m_index(_index)
        , m_count(_count)
        {};

        ~NodeConnector() = default;
        Node*              getNode()const;
        Node*              getConnectedNode() const;
        ImRect             getRect()const;
        ImVec2             getPos()const override;
        bool               connect(const NodeConnector *other) const override;
        virtual bool       hasSameParentWith(const NodeConnector *other) const override;

        static bool        Draw(const NodeConnector *_connector, const ImColor &_color, const ImColor &_hoveredColor);
        static void        DropBehavior(bool &needsANewNode);

        size_t    m_index;
        size_t    m_count;
        NodeView* m_nodeView;
        Way       m_way;
        const AppContext* m_context;
        static const NodeConnector* s_hovered;
        static const NodeConnector* s_dragged;
        static const NodeConnector* s_focused;

        static bool Connect(const NodeConnector *_left, const NodeConnector *_right);
    };
}