#include "ScopeView.h"
#include "tools/core/assertions.h"
#include "ndbl/core/Scope.h"
#include "NodeView.h"
#include "ndbl/core/Utils.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;

void ScopeView::init(Scope* scope)
{
    m_scope = scope;
    scope->set_view( this );

    if ( Scope* parent = scope->parent() )
        on_reset_parent( parent );
    for( Node* node : scope->child_node() )
        on_add_node( node );

    CONNECT( scope->on_add         , &ScopeView::on_add_node );
    CONNECT( scope->on_remove      , &ScopeView::on_remove_node );
    CONNECT( scope->on_reset_parent, &ScopeView::on_reset_parent );
}

Theme ScopeView::theme() const
{
    if ( m_scope->is_orphan() )
        return Theme_DARK;
    ScopeView* parent_view = m_scope->parent()->view();
    // use same theme since parent won't be drawn, contrast will be with parent's parent.
    if ( !parent_view->must_be_draw() )
        return parent_view->theme();
    // flip theme to maximize contrast
    return parent_view->theme() == Theme_LIGHT ? Theme_DARK
                                               : Theme_LIGHT;
}

void ScopeView::update(float dt, ScopeViewFlags flags)
{
    std::set<Node*> nodes { m_scope->child_node().begin(), m_scope->child_node().end() };
    if ( Scope::is_internal(m_scope) )
        nodes.insert( m_scope->node() );

    Rect r = {};
    m_nodeviews.clear();
    for(Node* node : nodes )
    {
        if (NodeView *nodeView = node->get_component<NodeView>())
        {
            m_nodeviews.push_back( nodeView );
            if (nodeView->visible())
            {
                Rect node_rect = nodeView->get_rect_ex(WORLD_SPACE, NodeViewFlag_WITH_RECURSION | NodeViewFlag_WITH_PINNED);
                r = Rect::merge(r, node_rect);
            }
        }

        if ( node->is_a_scope() )
        {
            for (Scope* child_scope: node->internal_scope()->child_scope())
            {
                child_scope->view()->update(dt, flags);
                r = Rect::merge(r, child_scope->view()->m_content_rect );
            };
        }
    }

    const Config* config = get_config();
    if ( r.has_area() )
    {
        r.min.x -= config->ui_scope_margin.x;
        r.min.y -= config->ui_scope_margin.y;
        r.max.x += config->ui_scope_margin.z;
        r.max.y += config->ui_scope_margin.w;

        r.min.round();
        r.max.round();
    }

    m_content_rect = r;
}

bool ScopeView::must_be_draw() const
{
    return m_content_rect.has_area() && m_nodeviews.size() >= 1;
}

void ScopeView::draw(float dt, bool highlight)
{
    if ( must_be_draw() )
    {
        const Rect r = m_content_rect;
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const Config* config = get_config();
        const Vec4& fill_col = theme() == Theme_DARK ? config->ui_scope_fill_col_light
                                                     : config->ui_scope_fill_col_dark;
        draw_list->AddRectFilled(r.min, r.max, ImGui::GetColorU32(fill_col), config->ui_scope_border_radius );
        if ( highlight )
        {
            draw_list->AddRect(r.min, r.max, ImGui::GetColorU32( config->ui_scope_border_col ) , config->ui_scope_border_radius, 0, config->ui_scope_border_thickness );
        }

        if ( ImGui::IsMouseHoveringRect(r.min, r.max) )
            on_hover.emit(this);
    }
}

void ScopeView::on_add_node(Node* node)
{
    if( NodeView* view = node->get_component<NodeView>())
    {
        m_spatial_node.add_child( &view->spatial_node() );
    }
}

void ScopeView::on_remove_node(Node* node)
{
    if( NodeView* view = node->get_component<NodeView>())
    {
        m_spatial_node.remove_child( &view->spatial_node() );
    }
}

void ScopeView::on_reset_parent(Scope* scope)
{
    if(m_spatial_node.parent() )
        m_spatial_node.parent()->remove_child(&m_spatial_node );

    if( scope )
        if( scope->view() )
            scope->view()->m_spatial_node.add_child( &m_spatial_node );
}

void ScopeView::translate(const Vec2 &delta)
{
    // translate scope's owner's view, only it this is the main internal scope
    if ( node()->internal_scope() == m_scope )
        if ( NodeView* owner_view = node()->get_component<NodeView>() )
            owner_view->spatial_node().translate( delta );
    // translate view (and children...)
    m_spatial_node.translate(delta );
}

void ScopeView::set_pinned(bool b)
{
    for ( NodeView* node_view : m_nodeviews )
        node_view->set_pinned( b );
    m_pinned = b;
}
