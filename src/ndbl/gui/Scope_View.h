#pragma once
#include "tools/gui/geometry/Rect.h"
#include "tools/gui/View_State.h"
#include "ndbl/core/Scope.h"

namespace ndbl
{
    // forward decl.
    class Node_View;

    typedef int Scope_View_Flags;
    enum Scope_View_Flag_
    {
        Scope_View_Flag_NONE          = 0,
        Scope_View_Flag_RECURSE       = 1 << 0,
        Scope_View_Flag_EXCLUDE_OWNER = 1 << 1,
    };

    typedef bool Theme;
    enum Theme_ : bool
    {
        Theme_DARK  = false,
        Theme_LIGHT = true
    };

    class Scope_View
    {
        using Rect = tools::Rect;
//== Data ==============================================================================================================
    public:
        tools::Signal<void(Scope_View*)> signal_hover;
    private:
        tools::View_State           m_view_state{};
        tools::Spatial_Node         m_spatial_node{};
        Scope*                      m_scope{};
        Rect                        m_content_rect{};
        std::vector<Node_View*>     m_wrapped_node_view{};
        Theme                       m_theme{};
//== Methods ===========================================================================================================
    public:
        Scope_View() = default;
        Scope_View(const Scope_View&) = default;

        void                init(Scope*);
        void                shutdown();
        void                update(float dt, Scope_View_Flags = Scope_View_Flag_NONE );
        void                draw(float dt);
        Node*               node() { return scope()->node(); }
        const Node*         node() const { return scope()->node(); }
        tools::View_State*  state() { return &m_view_state; }
        bool                has_parent() const { return parent() != nullptr; }
        Scope_View*         parent() const;
        Scope*              scope() const { return m_scope; }
        size_t              depth() const { return m_scope->depth(); }
        tools::Spatial_Node* spatial_node() { return &m_spatial_node; }
        void                set_position(const tools::Vec2& pos, tools::Space space ) { return m_spatial_node.set_position( pos, space );}
        void                translate(const tools::Vec2& delta) { m_spatial_node.translate(delta); }
        bool                must_be_draw() const;
        bool                pinned() const;
        void                set_pinned(bool b = true);
        const Rect&         content_rect() const { return m_content_rect; }
        void                arrange_content();
    };

    
    // extra ImGui-related tools
    
    void TreeNode_Scope(const char* title, ndbl::Scope*);
    void TreeNode_ScopeContent(ndbl::Scope*);
    void TreeNode_Node(ndbl::Node*);
}
