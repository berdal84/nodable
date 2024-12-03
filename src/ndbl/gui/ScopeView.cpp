#include "ScopeView.h"
#include "tools/core/assertions.h"
#include "ndbl/core/Scope.h"
#include "NodeView.h"
#include "ndbl/core/Utils.h"
#include "Config.h"
#include "Physics.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(ScopeView).extends<NodeComponent>();
)

void ScopeView::init(Scope* scope)
{
    m_scope = scope;
    scope->set_view( this );

    if ( Scope* parent = scope->parent() )
        on_reset_parent( parent );
    for( Node* node : scope->child() )
        on_add_node( node );

    CONNECT( scope->on_add         , &ScopeView::on_add_node     , this);
    CONNECT( scope->on_remove      , &ScopeView::on_remove_node  , this);
    CONNECT( scope->on_reset_parent, &ScopeView::on_reset_parent , this);
}

ScopeView* ScopeView::parent() const
{
    return m_scope->parent() ? m_scope->parent()->view() : nullptr;
}

void ScopeView::update(float dt, ScopeViewFlags flags)
{
    const Config* config = get_config();

    // 1) update recursively
    //    any scope with higher depth in the same hierarchy will be up to date.
    for( Node* child_node : m_scope->child() )
        if ( child_node->has_internal_scope() )
            child_node->internal_scope()->view()->update(dt, flags);

    for ( Scope* partition_scope: m_scope->partition() )
        partition_scope->view()->update(dt, flags);

    // 2) update content rectangle and wrapped node views
    //
    m_content_rect = {};
    m_wrapped_node_view.clear();
    auto wrap_nodeview = [&](NodeView* nodeview )
    {
        ASSERT( nodeview );
        if ( !nodeview->state().visible() )
            return;

        const NodeViewFlags nodeview_flags = NodeViewFlag_WITH_RECURSION
                                             | NodeViewFlag_WITH_PINNED;
        Rect node_rect = nodeview->get_rect_ex(WORLD_SPACE, nodeview_flags);
        m_content_rect = Rect::merge(m_content_rect, node_rect);
        m_wrapped_node_view.push_back(nodeview);
    };

    if ( !m_scope->is_partition() )
        if ( auto nodeview = m_scope->owner()->get_component<NodeView>() )
            wrap_nodeview( nodeview );

    for( Node* node : m_scope->child() )
        if ( auto nodeview = node->get_component<NodeView>() )
            wrap_nodeview( nodeview );

    for( Node* child_node : m_scope->child() )
    {
        if ( !child_node->has_internal_scope() )
            continue;

        ScopeView* child_node_scope_view = child_node->internal_scope()->view();
        child_node_scope_view->update(dt, flags);
        m_content_rect = Rect::merge(m_content_rect, child_node_scope_view->m_content_rect );
    }

    for ( Scope* partition_scope: m_scope->partition() )
    {
        ScopeView* partition_scope_view = partition_scope->view();
        partition_scope_view->update(dt, flags);
        m_content_rect = Rect::merge(m_content_rect, partition_scope_view->m_content_rect );
    };

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
    ScopeView* parent_view = parent();
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

bool ScopeView::must_be_draw() const
{
    if (!m_content_rect.has_area())
        return false;

    switch ( scope()->child().size() )
    {
        case 0:
            return false;
        case 1:
            if ( scope()->first_child()->has_internal_scope() && has_parent() )
                return false;
            return true;
        default:
            return true;
    }
}

void ScopeView::draw(float dt)
{
    if ( must_be_draw() )
    {
        const Rect r = m_content_rect;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const Config* config = get_config();
        const Vec4& fill_col = m_theme == Theme_DARK ? config->ui_scope_fill_col_light
                                                     : config->ui_scope_fill_col_dark;
        draw_list->AddRectFilled(r.min, r.max, ImGui::GetColorU32(fill_col), config->ui_scope_border_radius );
        if ( m_state.selected() )
        {
            draw_list->AddRect(r.min, r.max, ImGui::GetColorU32( config->ui_scope_border_col ) , config->ui_scope_border_radius, 0, config->ui_scope_border_thickness );
        }

        if ( ImGui::IsMouseHoveringRect(r.min, r.max) )
        {
            on_hover.emit(this);
        }
    }
}

void ScopeView::on_add_node(Node* node)
{
    if( NodeView* view = node->get_component<NodeView>())
    {
        m_state.spatial_node().add_child( &view->spatial_node() );
    }
}

void ScopeView::on_remove_node(Node* node)
{
    if( NodeView* view = node->get_component<NodeView>())
    {
        m_state.spatial_node().remove_child( &view->spatial_node() );
    }
}

void ScopeView::on_reset_parent(Scope* scope)
{
    if( m_state.spatial_node().has_parent() )
        m_state.spatial_node().parent()->remove_child(&m_state.spatial_node() );

    // this view must move when scope's owner view moves
    if( scope )
        scope->view()->m_state.spatial_node().add_child( &m_state.spatial_node() );
}

void ScopeView::translate(const tools::Vec2 &delta)
{
    // translate scope's owner's view, only it this is the main internal scope
    if ( node()->internal_scope() == m_scope )
        node()->get_component<NodeView>()->spatial_node().translate( delta );
    // translate view (and children...)
    m_state.spatial_node().translate( delta );
}

void ScopeView::set_pinned(bool b)
{
    node()->get_component<NodeView>()->state().set_pinned(b);
}

bool ScopeView::pinned() const
{
    return node()->get_component<NodeView>()->state().pinned();
}

void ScopeView::set_position(const tools::Vec2& pos, tools::Space space)
{
    m_state.spatial_node().set_position( pos, space );
}

void ScopeView::draw_scope_tree(Scope *scope)
{
    if ( ImGui::TreeNode("Scope Tree" ) )
    {
        if ( scope )
            draw_scope_tree_ex( scope );
        else
            ImGui::Text("nullptr");
        ImGui::TreePop();
    }
}

void ScopeView::draw_scope_tree_ex(Scope *scope)
{
    ImGui::PushID( scope );
    for ( Scope* sub_scope : scope->partition() )
    {
        draw_scope_tree_ex(sub_scope);
    }

    for ( Node* child : scope->child() )
    {
        ImGui::PushID(child);
        if ( ImGui::TreeNode("%s (primary)", child->get_class()->name(), child->name().c_str() ) )
        {
            if ( child->has_internal_scope() )
                draw_scope_tree_ex( child->internal_scope() );
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    ImGui::PopID();
}

void ScopeView::arrange_content()
{
    for( NodeView* view : m_wrapped_node_view )
    {
        view->arrange_recursively();
    }
}
