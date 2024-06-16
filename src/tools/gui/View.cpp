#include "View.h"
#include "ImGuiEx.h"

using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<View>("View");
}

View::View(View* parent)
: hovered(false)
, visible(true)
, selected(false)
, m_box()
, m_parent(parent)
{
}

void View::set_pos(const Vec2& p, Space space)
{
    if (space == PARENT_SPACE || m_parent == nullptr )
        return m_box.set_pos(p);

    Vec2 parent_space_pos = Vec2::transform(p, m_parent->m_box.model_matrix());
    m_box.set_pos(parent_space_pos);
}

Vec2 View::get_pos(Space space) const
{
    if (space == PARENT_SPACE || m_parent == nullptr )
        return m_box.get_pos();
    return Vec2::transform(m_box.get_pos(), m_parent->m_box.world_matrix() );
}

void View::translate(const Vec2& _delta)
{
    m_box.translate( _delta );
}

Rect View::get_rect(Space space) const
{
    if (space == PARENT_SPACE || m_parent == nullptr )
        return m_box.get_rect();

    // TODO: We should use Xform, but something is wrong with world/model matrices
    Rect result        = m_box.get_rect();
    Vec2 parent_origin = m_parent->get_rect(PARENT_SPACE).center();
    result.translate( parent_origin );
    return result;
}

void View::set_size(const Vec2& size)
{
    m_box.set_size(size );
}

Vec2 View::get_size() const
{
    return m_box.get_size();
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
        set_size(m_content_region.size());
        set_pos(m_content_region.center(), SCREEN_SPACE);
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
