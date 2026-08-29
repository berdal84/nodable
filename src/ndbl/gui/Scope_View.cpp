#include "Scope_View.h"
#include "bdc/String_Builder.hpp"
#include "core/Flags.h"
#include "gui/Layout.h"
#include "gui/View_Flags.h"
#include "tools/core/Asserts.h"
#include "ndbl/core/Scope.h"
#include "Node_View.h"
#include "Config.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

void ndbl::scopeview_init(Scope_View* scope_view, Scope* scope)
{
    ASSERT(scope != nullptr);

    scope_view->scope   = scope;
    scope->view         = scope_view;
}

void ndbl::scopeview_deinit(Scope_View* scope_view)
{
    scope_view->scope->view = nullptr;
    scope_view->scope       = nullptr;
}

Scope_View* ndbl::scopeview_get_parent(const Scope_View* scope_view)
{
    return scope_view->scope->parent ? scope_view->scope->parent->view : nullptr;
}

void ndbl::scopeview_update(Scope_View* scope_view, float dt, Scope_View_Flags flags)
{
    // 1) update recursively
    //    any scope with higher depth in the same hierarchy will be up to date.
    for( Node* child_node : scope_view->scope->children )
        if ( Scope* internal_scope = child_node->internal_scope )
            scopeview_update(internal_scope->view, dt, flags);

    // 2) update content rectangle and wrapped node views
    //
    scope_view->content_rect = {};
    scope_view->wrapped_node_view.clear();
    auto wrap_nodeview = [&](Node_View* nodeview )
    {
        ASSERT( nodeview );
        if ( HAS_FLAGS(nodeview->flags, View_Flag_HIDDEN) )
            return;

        const Rect r = nodeview_get_rect(nodeview, WORLD_SPACE);
        scope_view->content_rect = Rect::bounding_rect(scope_view->content_rect, r);
        scope_view->wrapped_node_view.push_back(nodeview);
    };

    // sibling nodeview is always wrapped inside its own scopeview
    if ( Node_View* sibling_nodeview = scope_view->scope->node->view )
        wrap_nodeview( sibling_nodeview );

    for( Node* node : scope_view->scope->children )
        if ( node->view )
            wrap_nodeview( node->view );

    for( Node* child_node : scope_view->scope->children )
    {
        if ( child_node->internal_scope )
        {
            Scope_View* child_node_scope_view = child_node->internal_scope->view;
            scopeview_update(child_node_scope_view, dt, flags);
            scope_view->content_rect = Rect::bounding_rect(scope_view->content_rect, child_node_scope_view->content_rect);
        }
    }

    if ( scopeview_must_be_draw(scope_view) )
    {
        // Add padding to clearly see nested scopes
        const Padding& pad = config()->ui_scope_padding;
        scope_view->content_rect.expand( pad.left, pad.top, pad.right, pad.bottom );

        // pixel perfect
        scope_view->content_rect.min.round();
        scope_view->content_rect.max.round();
    }


    // 2) update theme
    //
    if ( Scope_View* parent_view = scopeview_get_parent(scope_view) )
    {
        scope_view->theme = !parent_view->theme;
        if ( !scopeview_must_be_draw(parent_view) )
            scope_view->theme = !scope_view->theme;
    }
    else
    {
        scope_view->theme = Theme_DARK;
    }
}

bool ndbl::scopeview_must_be_draw(const Scope_View* scope_view)
{
    if (!scope_view->content_rect.has_area())
        return false;

    switch ( scope_view->scope->children.size() )
    {
        case 0:
            return false;
        case 1:
        {
            Node* single_node = *scope_view->scope->children.begin();
            if ( single_node->internal_scope != nullptr && scopeview_has_parent(scope_view) )
                return false;
            return true;
        }
        default:
            return true;
    }
}

void ndbl::scopeview_draw(Scope_View* scope_view, float dt)
{
    if ( !scopeview_must_be_draw(scope_view) )
        return;
    
    const Rect r = scope_view->content_rect;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const Vec4& fill_col = scope_view->theme == Theme_DARK
                         ? config()->ui_scope_fill_col_light
                         : config()->ui_scope_fill_col_dark;
    draw_list->AddRectFilled(r.min, r.max, ImGui::GetColorU32(fill_col), config()->ui_scope_border_radius );
    if ( HAS_FLAGS(scope_view->flags, View_Flag_SELECTED) )
    {
        draw_list->AddRect(r.min, r.max, ImGui::GetColorU32( config()->ui_scope_border_col ) , config()->ui_scope_border_radius, 0, config()->ui_scope_border_thickness );
    }

    if ( ImGui::IsMouseHoveringRect(r.min, r.max) )
    {
        scope_view->signal_hover.emit(scope_view);
    }
}

void ndbl::scopeview_arrange_content(Scope_View* scope_view)
{
    for( Node_View* view : scope_view->wrapped_node_view )
    {
        nodeview_arrange_recursively(view);
    }
}

void ndbl::TreeNode_Scope(const bdc::String& title, Scope* scope)
{
    if ( ImGui::TreeNode( title.c_str() ) )
    {
        if ( scope )
            TreeNode_ScopeContent(scope);
        else
            ImGui::Text("nullptr");
        ImGui::TreePop();
    }
}

void ndbl::TreeNode_Node(Node* node)
{
    bool open = false;
    switch ( node->type )
    {
        case Node_Type_OPERATOR:
        case Node_Type_FUNCTION:
        {
            bdc::String_Builder sb;
            lang_serialize_func_sig(language(), sb, &node->invokable_data.type);
            char str[255];
            open = ImGui::TreeNode(node, "[%p] \"%s\" (%s, %s)", node, node->name.c_str(), node->get_class()->name.c_str(), bdc::string_builder_build_string(sb).c_str() );
            break;
        }
        case Node_Type_VARIABLE:
        {
            open = ImGui::TreeNode(node, "[%p] \"%s\" (%s)", node, node->value->token.word_view.c_str(), node->name.c_str());
            break;
        }
        default:
        {
            open = ImGui::TreeNode(node, "[%p] \"%s\" (%s)", node, node->name.c_str(), node->get_class()->name.c_str() );
        }
    }

    if ( open )
    {
        if ( node->internal_scope != nullptr )
        {
            TreeNode_ScopeContent(node->internal_scope );
        }

        ImGui::TreePop();
    }
};

void ndbl::TreeNode_ScopeContent(Scope *scope)
{
    ImGui::PushID( scope );
    std::vector<Node*> backbone = scope_get_backbone(scope);
    if ( ImGui::TreeNodeEx(&backbone, ImGuiTreeNodeFlags_DefaultOpen, "Children (backbone, ordered)" ) )
    {
        for ( Node* each_node : backbone )
        {
            TreeNode_Node(each_node);
        }
        ImGui::TreePop();
    }

    if ( ImGui::TreeNode(&scope->variables, "Children (vars only, unordered)") )
    {
        for ( Node* each_node : scope->variables )
        {
            TreeNode_Node(each_node);
        }
        ImGui::TreePop();
    }

    if ( ImGui::TreeNode(&scope->children, "Children (all, unordered)") )
    {
        for ( Node* each_node : scope->children )
        {
            TreeNode_Node(each_node);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}
