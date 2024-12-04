#pragma once
#include "tools/core/Component.h"
#include "tools/gui/geometry/Rect.h"
#include "tools/gui/geometry/BoxShape2D.h"
#include "tools/gui/ViewState.h"
#include "ndbl/core/ASTScope.h"

namespace ndbl
{
    // forward decl.
    class ASTNodeView;

    typedef int ScopeViewFlags;
    enum ScopeViewFlags_
    {
        ScopeViewFlags_NONE          = 0,
        ScopeViewFlags_RECURSE       = 1 << 0,
        ScopeViewFlags_EXCLUDE_OWNER = 1 << 1,
    };

    typedef bool Theme;
    enum Theme_ : bool
    {
        Theme_DARK  = false,
        Theme_LIGHT = true
    };

    class ScopeView : public tools::Component
    {
    public:
        DECLARE_REFLECT_override
        typedef tools::Rect Rect;

        SIGNAL(on_hover, ScopeView*);

        void         init(ASTScope*);
        void         update(float nodeview, ScopeViewFlags flags = ScopeViewFlags_NONE );
        void         draw(float dt);
        tools::ViewState& state() { return m_state; }
        bool         has_parent() const { return m_scope->parent() != nullptr; }
        ScopeView*   parent() const;
        ASTScope*       scope() const { return m_scope; }
        size_t       depth() const { return m_scope->depth(); }
        void         set_position(const tools::Vec2& pos, tools::Space space);
        void         translate(const tools::Vec2 &vec2);
        bool         must_be_draw() const;
        bool         pinned() const;
        void         set_pinned(bool b = true);
        const Rect&  content_rect() const { return m_content_rect; }
        void         arrange_content();
        const tools::SpatialNode2D& spatial_node() const { return m_state.spatial_node(); }
        tools::SpatialNode2D&       spatial_node() { return m_state.spatial_node(); }

        static void  draw_scope_tree(ASTScope* scope);

    private:
        static void  draw_scope_tree_ex(ASTScope* scope);
        void        on_reset_parent(ASTScope*);
        void        on_add_node(ASTNode*);
        void        on_remove_node(ASTNode*);

        tools::ViewState     m_state;
        ASTScope*               m_scope = nullptr;
        Rect                 m_content_rect;
        std::vector<ASTNodeView*> m_wrapped_node_view;
        Theme                m_theme;
    };
}