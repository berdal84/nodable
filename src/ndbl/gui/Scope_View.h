#pragma once
#include "gui/geometry/Spatial_Node.h"
#include "tools/gui/geometry/Rect.h"
#include "tools/gui/View_Flags.h"
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

    struct Scope_View
    {
        tools::Signal<void(Scope_View*)>    signal_hover        = {};
        tools::Spatial_Node                 spatial_node        = {};
        std::vector<Node_View*>             wrapped_node_view   = {};
        tools::Rect                         content_rect        = {};
        tools::View_Flags                   flags               = tools::View_Flag_DEFAULTS;
        Scope*                              scope               = {};
        Theme                               theme               = {};
    };    

    void                            scopeview_init(Scope_View*, Scope*);
    void                            scopeview_deinit(Scope_View*);
    void                            scopeview_update(Scope_View*, float dt, Scope_View_Flags = Scope_View_Flag_NONE );
    void                            scopeview_draw(Scope_View*, float dt);
    inline bool                     scopeview_has_parent(const Scope_View* scope_view) { return scope_view->scope->parent != nullptr; }
    Scope_View*                     scopeview_get_parent(const Scope_View* scope_view);
    inline size_t                   scopeview_get_depth(const Scope_View* scope_view) { return scope_get_depth(scope_view->scope); }
    bool                            scopeview_must_be_draw(const Scope_View* );
    void                            scopeview_arrange_content(Scope_View*);

    // extra ImGui-related tools
    
    void TreeNode_Scope(const char* title, ndbl::Scope*);
    void TreeNode_ScopeContent(ndbl::Scope*);
    void TreeNode_Node(ndbl::Node*);
}
