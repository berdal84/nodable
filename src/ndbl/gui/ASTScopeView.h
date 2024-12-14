#pragma once
#include "tools/core/ComponentFor.h"
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

    class ASTScopeView : public tools::ComponentFor<ASTNode>
    {
    public:
        DECLARE_REFLECT_override
        typedef tools::Rect Rect;

        SIGNAL(on_hover, ASTScopeView*);

        ASTScopeView(): tools::ComponentFor<ASTNode>("ScopeView") {}

        void         init(ASTScope*, tools::SpatialNode*);
        void         update(float dt, ScopeViewFlags = ScopeViewFlags_NONE );
        void         draw(float dt);
        tools::ViewState* state() { return &m_view_state; }
        bool         has_parent() const { return m_scope->parent() != nullptr; }
        ASTScopeView*parent() const;
        ASTScope*    scope() const { return m_scope; }
        size_t       depth() const { return m_scope->depth(); }
        void         set_position(const tools::Vec2& pos, tools::Space space ) { return m_spatial_node->set_position( pos, space );}
        void         translate(const tools::Vec2& delta) { m_spatial_node->translate(delta); }
        bool         must_be_draw() const;
        bool         pinned() const;
        void         set_pinned(bool b = true);
        const Rect&  content_rect() const { return m_content_rect; }
        void         arrange_content();

        static void  ImGuiTreeNode_ASTScope(const char* title, ASTScope*);

    private:
        static void  ImGuiTreeNode_ASTScopeContent(ASTScope*);
        static void  ImGuiTreeNode_ASTNode(ASTNode*);
        void         on_add_node(ASTNode*);
        void         on_remove_node(ASTNode*);

        tools::ViewState          m_view_state;
        tools::SpatialNode*       m_spatial_node = nullptr;
        ASTScope*                 m_scope = nullptr;
        Rect                      m_content_rect;
        std::vector<ASTNodeView*> m_wrapped_node_view;
        Theme                     m_theme;
    };
}