#pragma once
#include "tools/gui/geometry/Rect.h"
#include "tools/gui/geometry/BoxShape2D.h"
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

    enum Theme
    {
        Theme_DARK,
        Theme_LIGHT
    };

    class ScopeView : public NodeComponent
    {
    public:
        DECLARE_REFLECT_override
        typedef tools::Rect Rect;

        SIGNAL(on_hover, ScopeView*);

        void         init(Scope*);
        void         update(float dt, ScopeViewFlags flags = ScopeViewFlags_NONE );
        void         draw(float dt, bool highlight);
        Scope*       scope() const { return m_scope; }
        size_t       depth() const { return m_scope->depth(); }
        Theme        theme() const;
        void         set_position(const tools::Vec2& pos, tools::Space space);
        void         translate(const tools::Vec2 &vec2);
        bool         must_be_draw() const;
        bool         pinned() const;
        void         set_pinned(bool b = true);
        const Rect&  content_rect() const { return m_content_rect; }
        static void  draw_scope_tree(Scope* scope);
    private:
        static void  draw_scope_tree_ex(Scope* scope);
        void        on_reset_parent(Scope*);
        void        on_add_node(Node*);
        void        on_remove_node(Node*);

        Scope*               m_scope = nullptr;
        tools::SpatialNode2D m_spatial_node;
        Rect                 m_content_rect;
        std::vector<NodeView*> m_inner_nodeviews;
    };
}