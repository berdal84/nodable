#include "ScopeView.h"
#include "tools/core/assertions.h"
#include "ndbl/core/Scope.h"
#include "NodeView.h"
#include "ndbl/core/Utils.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;


ScopeView::ScopeView(Scope* scope)
: NodeComponent()
{
    m_scope = scope;
    scope->set_view( this );
}

void ScopeView::update(float dt, ScopeViewFlags flags)
{
    // Strategy:
    // 1) reset current rect to a null area
    // 2) gather all child node views and compute the bbox
    // 3) recursively update and get child scope bbox

    // 1)
    m_rect = {};

    // 2)
    std::vector<NodeView*> node_views;
    for(Node* node : m_scope->child_node() )
        if( NodeView* node_view = node->get_component<NodeView>() )
            node_views.push_back(node_view );

    // Include owner's view if this scope is its inner scope
    Node* owner = m_scope->get_owner();
    if( owner->inner_scope() == m_scope )
        node_views.push_back( owner->get_component<NodeView>() );

    // grab all nodes's bbox
    m_rect = NodeView::get_rect(node_views, WORLD_SPACE, NodeViewFlag_WITH_RECURSION | NodeViewFlag_WITH_PINNED );

    // 3)
    if ( (flags & ScopeFlags_RECURSE) == 0 )
        return;

    const Config* config = get_config();
    for (Scope* child : m_scope->child_scope() )
    {
        ScopeView* child_view = child->view();

        if ( !child_view )
            continue;

        child_view->update( dt, flags );

        if( !child_view->m_rect.has_area() )
            continue;

        m_rect.expand( config->ui_scope_child_margin );
        m_rect = !m_rect.has_area() ? child_view->m_rect
                                    : Rect::merge( m_rect, child_view->m_rect );
    }

    if ( m_rect.has_area() )
    {
        m_rect.min.x -= config->ui_scope_margin.x;
        m_rect.min.y -= config->ui_scope_margin.y;
        m_rect.max.x += config->ui_scope_margin.z;
        m_rect.max.y += config->ui_scope_margin.w;
    }
}

void ScopeView::draw(float dt)
{
    if ( m_rect.has_area() )
    {
        Rect r = m_rect;
        r.min.round();
        r.max.round();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const Config* config = get_config();
        draw_list->AddRectFilled(r.min, r.max, ImGui::GetColorU32( config->ui_scope_fill_col ), config->ui_scope_border_radius );
        draw_list->AddRect      (r.min, r.max, ImGui::GetColorU32( config->ui_scope_border_col ), config->ui_scope_border_radius, 0, config->ui_scope_border_thickness );
    }
}
