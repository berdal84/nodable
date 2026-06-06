#include "Scope_View.h"
#include "tools/core/Asserts.h"
#include "ndbl/core/Scope.h"
#include "Node_View.h"
#include "Config.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

void Scope_View::init(Scope* scope)
{
    ASSERT(scope != nullptr);

    m_scope = scope;
    scope->set_view(this);
}

void Scope_View::shutdown()
{
    spatial_node()->clear();
    m_scope->set_view(nullptr);
    m_scope = nullptr;
}

Scope_View* Scope_View::parent() const
{
    return m_scope->parent() ? m_scope->parent()->view() : nullptr;
}

void Scope_View::update(float dt, Scope_View_Flags flags)
{
    const Config* config = get_config();

    // 1) update recursively
    //    any scope with higher depth in the same hierarchy will be up to date.
    for( Node* child_node : m_scope->children() )
        if ( Scope* internal_scope = child_node->internal_scope() )
            internal_scope->view()->update(dt, flags);

    // 2) update content rectangle and wrapped node views
    //
    m_content_rect = {};
    m_wrapped_node_view.clear();
    auto wrap_nodeview = [&](Node_View* nodeview )
    {
        ASSERT( nodeview );
        if ( !nodeview->state()->visible() )
            return;

        const Rect r = nodeview->get_rect(WORLD_SPACE);
        m_content_rect = Rect::bounding_rect(m_content_rect, r);
        m_wrapped_node_view.push_back(nodeview);
    };

    // sibling nodeview is always wrapped inside its own scopeview
    if ( auto sibling_nodeview = m_scope->node()->component<Node_View>() )
        wrap_nodeview( sibling_nodeview );

    for( Node* node : m_scope->children() )
        if ( auto nodeview = node->component<Node_View>() )
            wrap_nodeview( nodeview );

    for( Node* child_node : m_scope->children() )
    {
        if ( child_node->has_internal_scope() )
        {
            Scope_View* child_node_scope_view = child_node->internal_scope()->view();
            child_node_scope_view->update(dt, flags);
            m_content_rect = Rect::bounding_rect(m_content_rect, child_node_scope_view->m_content_rect);
        }
    }

    if ( must_be_draw() )
    {
        // Add margins to see clearly nested scopes
        m_content_rect.min -= config->ui_scope_content_rect_margin.min;
        m_content_rect.max += config->ui_scope_content_rect_margin.max;

        // pixel perfect
        m_content_rect.min.round();
        m_content_rect.max.round();
    }


    // 2) update theme
    //
    Scope_View* parent_view = parent();
    if ( parent_view )
    {
        m_theme = !parent_view->m_theme;
        if ( !parent_view->must_be_draw() )
            m_theme = !m_theme;
    }
    else
    {
        m_theme = Theme_DARK;
    }
}

bool Scope_View::must_be_draw() const
{
    if (!m_content_rect.has_area())
        return false;

    switch ( scope()->children().size() )
    {
        case 0:
            return false;
        case 1:
        {
            Node* single_node = *scope()->children().begin();
            if ( single_node->has_internal_scope() && this->has_parent() )
                return false;
            return true;
        }
        default:
            return true;
    }
}

void Scope_View::draw(float dt)
{
    if ( must_be_draw() )
    {
        const Rect r = m_content_rect;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const Config* config = get_config();
        const Vec4& fill_col = m_theme == Theme_DARK ? config->ui_scope_fill_col_light
                                                     : config->ui_scope_fill_col_dark;
        draw_list->AddRectFilled(r.min, r.max, ImGui::GetColorU32(fill_col), config->ui_scope_border_radius );
        if ( m_view_state.selected() )
        {
            draw_list->AddRect(r.min, r.max, ImGui::GetColorU32( config->ui_scope_border_col ) , config->ui_scope_border_radius, 0, config->ui_scope_border_thickness );
        }

        if ( ImGui::IsMouseHoveringRect(r.min, r.max) )
        {
            signal_hover.emit(this);
        }
    }
}

void Scope_View::set_pinned(bool b)
{
    m_view_state.set_pinned(b);
}

bool Scope_View::pinned() const
{
    return m_view_state.pinned();
}

void Scope_View::arrange_content()
{
    for( Node_View* view : m_wrapped_node_view )
    {
        view->arrange_recursively();
    }
}

void ndbl::TreeNode_Scope(const char* title, Scope* scope)
{
    if ( ImGui::TreeNode( title ) )
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
    switch ( node->type() )
    {
        case Node_Type_OPERATOR:
        case Node_Type_FUNCTION:
        {
            std::string signature;
            get_language()->serialize_func_sig(signature, node->invokable_data().get_func_type());
            char str[255];
            open = ImGui::TreeNode(node, "[%p] \"%s\" (%s, %s)", node, node->name().c_str(), node->get_class()->name(), signature.c_str());
            break;
        }
        case Node_Type_VARIABLE:
        {
            std::string value = node->value()->token().word_to_string();
            char str[255];
            open = ImGui::TreeNode(node, "[%p] \"%s\" (%s)", node, value.c_str(), node->name().c_str());
            break;
        }
        default:
        {
            open = ImGui::TreeNode(node, "[%p] \"%s\" (%s)", node, node->name().c_str(), node->get_class()->name());
        }
    }

    if ( open )
    {
        if ( node->has_internal_scope() )
        {
            TreeNode_ScopeContent(node->internal_scope());
        }

        ImGui::TreePop();
    }
};

void ndbl::TreeNode_ScopeContent(Scope *scope)
{
    ImGui::PushID( scope );
    std::vector<Node*> backbone = scope->backbone();
    if ( ImGui::TreeNodeEx(&backbone, ImGuiTreeNodeFlags_DefaultOpen, "Children (backbone, ordered)" ) )
    {
        for ( Node* each_node : backbone )
        {
            TreeNode_Node(each_node);
        }
        ImGui::TreePop();
    }

    if ( ImGui::TreeNode(&scope->variables(), "Children (vars only, unordered)") )
    {
        for ( Node* each_node : scope->variables() )
        {
            TreeNode_Node(each_node);
        }
        ImGui::TreePop();
    }

    if ( ImGui::TreeNode(&scope->children(), "Children (all, unordered)") )
    {
        for ( Node* each_node : scope->children() )
        {
            TreeNode_Node(each_node);
        }
        ImGui::TreePop();
    }
    ImGui::PopID();
}
