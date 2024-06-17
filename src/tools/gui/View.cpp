#include "View.h"
#include "ImGuiEx.h"

using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<View>("View");
}

View::View()
: hovered(false)
, visible(true)
, selected(false)
, m_screen_box()
, m_parent(nullptr)
{
}

void View::set_pos(const Vec2& p, Space space)
{
    const Vec2 old_pos = m_screen_box.get_pos();
    if (space == SCREEN_SPACE || m_parent == nullptr )
    {
        m_screen_box.set_pos(p);
    }
    else
    {
        Vec2 screen_space_pos = Vec2::transform(p, m_parent->m_screen_box.world_matrix());
        m_screen_box.set_pos(screen_space_pos);
    }

    if ( m_children.empty() )
        return;

    const Vec2 delta = m_screen_box.get_pos() - old_pos;
    for(View* child : m_children)
        child->translate(delta);
}

Vec2 View::get_pos(Space space) const
{
    if (space == SCREEN_SPACE || m_parent == nullptr )
        return m_screen_box.get_pos();
    return Vec2::transform(m_screen_box.get_pos(), m_parent->m_screen_box.model_matrix() );
}

void View::translate(const Vec2& _delta)
{
    set_pos(get_pos(SCREEN_SPACE) + _delta, SCREEN_SPACE );
}

Rect View::get_rect(Space space) const
{
    if (space == SCREEN_SPACE || m_parent == nullptr )
        return m_screen_box.get_rect();

   Box2D parent_space_box = Box2D::transform(m_screen_box, m_parent->m_screen_box.model_matrix() );
   return parent_space_box.get_rect();
}

void View::set_size(const Vec2& size)
{
    m_screen_box.set_size(size);
}

Vec2 View::get_size() const
{
    return m_screen_box.get_size();
}

View* View::get_parent() const
{
    return m_parent;
}

bool View::draw()
{
    m_content_region = ImGuiEx::GetContentRegion(SCREEN_SPACE);

    if ( m_parent == nullptr)
    {
        m_screen_box.set_size( m_content_region.size() );
        m_screen_box.set_pos( m_content_region.center() ); // do not replace by this->set_pos(...)
    }

#ifdef TOOLS_DEBUG
    Rect r = get_rect(SCREEN_SPACE);
    if ( r.size().lensqr() < 0.1f )
    {
        r = m_content_region;
    }
    ImGuiEx::DebugRect(r.min, r.max, ImColor(255, 0, 0));     // box
    ImGuiEx::DebugLine(r.tl(), r.br(), ImColor(255, 0,0, 127));    // diagonal 1
    ImGuiEx::DebugLine(r.bl(), r.tr(), ImColor(255, 0,0, 127 ));    // diagonal 2
    ImGuiEx::DebugCircle(r.center(), 2.f, ImColor(255, 0,0)); // center

    // center to parent center
    if ( m_parent != nullptr)
         ImGuiEx::DebugLine(m_parent->get_pos(SCREEN_SPACE), r.center(), ImColor(255, 0,255, 127 ), 4.f);
#endif
    return false;
}

const Rect &View::get_content_region(Space space) const
{
    ASSERT(space == SCREEN_SPACE) // Only space handled
    return m_content_region;
}

void View::add_child(View* view)
{
    m_children.push_back(view);
    view->m_parent = this;
}
