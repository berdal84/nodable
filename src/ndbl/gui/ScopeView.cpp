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
    if ( is_owner() )
        nodes.insert( m_scope->get_owner() );

    m_rect = {};
    m_nodeviews.clear();
    for(Node* node : nodes )
    {
        if (NodeView *nodeView = node->get_component<NodeView>())
        {
            m_nodeviews.push_back( nodeView );
            if (nodeView->visible())
            {
                Rect node_rect = nodeView->get_rect_ex(WORLD_SPACE, NodeViewFlag_WITH_RECURSION | NodeViewFlag_WITH_PINNED);
                m_rect = Rect::merge(m_rect, node_rect);
            }
        }

        if ( node->is_a_scope() )
        {
            for (Scope* child_scope: node->internal_scope()->child_scope())
            {
                child_scope->view()->update(dt, flags);
                m_rect = Rect::merge(m_rect, child_scope->view()->m_rect);
            };
        }
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

bool ScopeView::must_be_draw() const
{
    return m_rect.has_area() && m_nodeviews.size() >= 1;
}

void ScopeView::draw(float dt, bool highlight)
{
    if ( must_be_draw() )
    {
        Rect r = m_rect;
        r.min.round();
        r.max.round();

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

bool ScopeView::is_owner() const
{
    return m_scope->get_owner()->is_a_scope()
        && m_scope->get_owner()->internal_scope() == m_scope;
}

void ScopeView::translate(const Vec2 &delta , ScopeViewFlags flags)
{
    // translate direct nodes
    for (NodeView* views : m_nodeviews )
        if ( (flags & ScopeViewFlags_EXCLUDE_OWNER) == 0 || views->get_owner() != this->get_owner() )
            views->xform()->translate(delta);

    // translate indirect nodes
    for (Scope* scope : m_scope->child_scope() )
        scope->view()->translate(delta, ScopeViewFlags_EXCLUDE_OWNER );
}

