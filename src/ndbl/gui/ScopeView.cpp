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

Rect ScopeView::update_node(float dt, ScopeViewFlags flags, Node* node)
{
    Rect r;

    if ( NodeView *node_view = node->get_component<NodeView>() )
    {
        if ( node_view->visible() )
            r = node_view->get_rect_ex(WORLD_SPACE, NodeViewFlag_WITH_RECURSION | NodeViewFlag_WITH_PINNED );
    }

    if ( node->is_a_scope() )
    {
        for(Scope* s : node->internal_scope()->child_scope())
        {
            s->view()->update(dt, flags);
            r = !r.has_area() ? s->view()->m_rect
                              : Rect::merge( r, s->view()->m_rect );
        };
    }
    return r;
}
void ScopeView::update(float dt, ScopeViewFlags flags)
{
    std::set<Node*> nodes { m_scope->child_node().begin(), m_scope->child_node().end() };

    if ( is_owner() )
    {
        nodes.insert( m_scope->get_owner() );
    }

    m_rect = {};
    for(Node* node : nodes )
    {
        Rect r = update_node(dt, flags, node );
        if ( !m_rect.has_area() )
            m_rect = r;
        else
            m_rect = Rect::merge( m_rect, r );
    }

    const Config* config = get_config();
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

bool ScopeView::is_owner() const
{
    return m_scope->get_owner()->is_a_scope()
        && m_scope->get_owner()->internal_scope() == m_scope;
}
