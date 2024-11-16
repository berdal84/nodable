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
    m_inner_nodeviews.clear();
    std::set<Node*> node_to_include {m_scope->child().begin(), m_scope->child().end() };

    // Insert owner if this scope is the main internal one
    if ( m_scope->node()->internal_scope() == m_scope )
      node_to_include.insert(m_scope->node() );

    Rect content_rect = {};

    // append node's view rectangle
    for( Node* node : node_to_include )
    {
        if (NodeView *nodeView = node->get_component<NodeView>())
        {
            m_inner_nodeviews.push_back( nodeView );
            if (nodeView->visible())
            {
                Rect node_rect = nodeView->get_rect_ex(WORLD_SPACE, NodeViewFlag_WITH_RECURSION | NodeViewFlag_WITH_PINNED);
                content_rect = Rect::merge(content_rect, node_rect);
            }
        }
    }

    // append child's internal scope rectangle(s)
    for( Node* node : m_scope->child() )
    {
        if ( !node->has_internal_scope() )
            continue;

        m_inner_nodeviews.push_back( node->get_component<NodeView>() );
        Scope* scope = node->internal_scope();
        scope->view()->update(dt, flags);
        content_rect = Rect::merge(content_rect, scope->view()->m_content_rect );
    }

    // append sub scope rectangle(s)
    for ( Scope* sub_scope: m_scope->sub_scope() )
    {
        ScopeView* child_scope_view = sub_scope->view();
        child_scope_view->update(dt, flags);
        content_rect = Rect::merge(content_rect, child_scope_view->m_content_rect );
    };

    const Config* config = get_config();
    if ( content_rect.has_area() )
    {
        content_rect.min.x -= config->ui_scope_margin.x;
        content_rect.min.y -= config->ui_scope_margin.y;
        content_rect.max.x += config->ui_scope_margin.z;
        content_rect.max.y += config->ui_scope_margin.w;

        content_rect.min.round();
        content_rect.max.round();
    }

    m_content_rect = content_rect;
}

bool ScopeView::must_be_draw() const
{
    return m_content_rect.has_area() && m_inner_nodeviews.size() >= 1;
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
    if( m_spatial_node.has_parent() )
        m_spatial_node.parent()->remove_child(&m_spatial_node );

    // this view must move when scope's owner view moves
    if( scope )
        scope->view()->m_spatial_node.add_child( &m_spatial_node );
}

void ScopeView::translate(const tools::Vec2 &delta)
{
    // translate scope's owner's view, only it this is the main internal scope
    if ( node()->internal_scope() == m_scope )
        node()->get_component<NodeView>()->spatial_node().translate( delta );
    // translate view (and children...)
    m_spatial_node.translate( delta );
}

void ScopeView::set_pinned(bool b)
{
    node()->get_component<NodeView>()->set_pinned(b);
}

bool ScopeView::pinned() const
{
    return node()->get_component<NodeView>()->pinned();
}

void ScopeView::set_position(const tools::Vec2& pos, tools::Space space)
{
    m_spatial_node.set_position( pos, space );
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
    for ( Scope* sub_scope : scope->sub_scope() )
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
