#pragma once
#include "tools/gui/geometry/Rect.h"
#include "ndbl/core/NodeComponent.h"
#include "ndbl/core/Scope.h"

namespace ndbl
{
    // forward decl.
    class NodeView;

    typedef int ScopeViewFlags;
    enum ScopeViewFlags_
    {
        ScopeViewFlags_NONE          = 0,
        ScopeViewFlags_RECURSE       = 1 << 0,
        ScopeViewFlags_EXCLUDE_OWNER = 1 << 1,
    };

    class ScopeView : public NodeComponent
    {
    public:
        typedef tools::Rect Rect;

        ScopeView(Scope* scope);
        void         update(float dt, ScopeViewFlags flags = ScopeViewFlags_NONE );
        void         draw(float dt);
        Scope*       scope() const { return m_scope; }
        const Rect&  rect() const { return m_rect; }
        bool         hovered() const { return m_hovered; }
        bool         dragged() const { return m_dragged; }
        const std::vector<NodeView*> &nodeviews() const { return m_nodeviews; }
        void         translate(const tools::Vec2 &delta, ScopeViewFlags flags = ScopeViewFlags_NONE);
        size_t       depth() const { return m_scope->depth(); }

    private:
        bool        is_owner() const;

        Rect        m_rect;
        Scope*      m_scope;
        bool        m_dragged;
        bool        m_hovered;
        std::vector<NodeView*> m_nodeviews;
    };
}