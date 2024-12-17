#pragma once
#include "tools/core/Component.h"
#include "tools/gui/geometry/Rect.h"
#include "tools/gui/geometry/BoxShape2D.h"
#include "tools/gui/ViewState.h"
#include "ndbl/core/ASTScope.h"
#include "ndbl/gui/concepts/CView.h"

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

    class ASTScopeView : public tools::Component<ASTNode>
    {
    public:
        DECLARE_REFLECT_override
        typedef tools::Rect Rect;

        tools::Signal<void(ASTScopeView*)> on_hover;

        ASTScopeView() = delete;
        ASTScopeView(ASTScope*);

        void         update(float dt, ScopeViewFlags = ScopeViewFlags_NONE );
        void         draw(float dt);
        tools::ViewState* state() { return &m_view_state; }
        bool         has_parent() const { return m_scope->parent() != nullptr; }
        ASTScopeView*parent() const;
        ASTScope*    scope() const { return m_scope; }
        size_t       depth() const { return m_scope->depth(); }
        tools::SpatialNode* spatial_node() { return &m_spatial_node; }
        void         set_position(const tools::Vec2& pos, tools::Space space ) { return m_spatial_node.set_position( pos, space );}
        void         translate(const tools::Vec2& delta) { m_spatial_node.translate(delta); }
        bool         must_be_draw() const;
        bool         pinned() const;
        void         set_pinned(bool b = true);
        const Rect&  content_rect() const { return m_content_rect; }
        void         arrange_content();
        void         add_child(CView auto* child);
        void         remove_child(CView auto* child);
    private:
        void         _on_shutdown();
        void         _on_add_node(ASTNode*);
        void         _on_remove_node(ASTNode*);
    public:
        static void  ImGuiTreeNode_ASTScope(const char* title, ASTScope*);
    private:
        static void  ImGuiTreeNode_ASTScopeContent(ASTScope*);
        static void  ImGuiTreeNode_ASTNode(ASTNode*);

        tools::ViewState          m_view_state;
        tools::SpatialNode        m_spatial_node;
        ASTScope*                 m_scope = nullptr;
        Rect                      m_content_rect;
        std::vector<ASTNodeView*> m_wrapped_node_view;
        Theme                     m_theme;
    };

    void ASTScopeView::add_child(CView auto* child)
    {
        spatial_node()->add_child( child->spatial_node() );
        child->spatial_node()->set_position({0.f, 0.f}, tools::PARENT_SPACE);
    }

    void ASTScopeView::remove_child(CView auto* child)
    {
        spatial_node()->remove_child( child->spatial_node() );
    }
}