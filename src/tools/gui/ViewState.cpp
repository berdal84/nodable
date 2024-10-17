#include "ViewState.h"
#include "ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_DRAW 0
#endif

using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ViewState>("View");
}

ViewState::ViewState()
: hovered(false)
, visible(true)
, selected(false)
, m_box()
, m_parent(nullptr)
{
}

void ViewState::set_pos(const Vec2& p, Space space)
{
    const Vec2 old_pos = m_box.get_pos();

    switch ( space )
    {
        case PARENT_SPACE:
        {
            if ( m_parent == nullptr)
            {
                m_box.set_pos(p);
                break;
            }
            Vec2 parent_space_pos = Vec2::transform(p, m_parent->m_box.world_matrix());
            m_box.set_pos(parent_space_pos );
            break;
        }
        case SCREEN_SPACE:
            return set_pos(p - m_window_pos, PARENT_SPACE);
        case LOCAL_SPACE:
            return set_pos(get_pos(PARENT_SPACE) + p, PARENT_SPACE);
    }

    if ( m_children.empty() )
        return;

    const Vec2 delta = m_box.get_pos() - old_pos;
    for(ViewState* child : m_children)
        child->translate(delta); // TODO: use isDirty pattern instead. Positions must be relative to parent.
}

Vec2 ViewState::get_pos(Space space) const
{
    switch (space)
    {
        case LOCAL_SPACE:
            return m_box.get_pos();
        case PARENT_SPACE:
            if ( m_parent == nullptr )
                return m_box.get_pos();
            return Vec2::transform(
                    m_box.get_pos(),
                    m_parent->m_box.model_matrix());
        case SCREEN_SPACE:
            return m_window_pos + get_pos(PARENT_SPACE);
        default:
            ASSERT(false) // not handled yet
    }
}

void ViewState::translate(const Vec2& _delta)
{
    Vec2 new_pos = get_pos( PARENT_SPACE ) + _delta;
    set_pos( new_pos, PARENT_SPACE);
}

Rect ViewState::get_rect(Space space) const
{
    switch (space)
    {
        case LOCAL_SPACE:
            return m_box.get_rect();
        case PARENT_SPACE:
        {
            if (m_parent == nullptr)
                return m_box.get_rect();
            Box parent_space_box = Box::transform(m_box, m_parent->m_box.model_matrix());
            return parent_space_box.get_rect();
        }
        case SCREEN_SPACE:
        {
            Rect r = get_rect(PARENT_SPACE);
            return {
                m_window_pos + r.min,
                m_window_pos + r.max,
            };
        }
        default:
            ASSERT(false) // Not implemented yet
    }
}

void ViewState::set_size(const Vec2& size)
{
    m_box.set_size(size);
}

Vec2 ViewState::get_size() const
{
    return m_box.get_size();
}

ViewState* ViewState::get_parent() const
{
    return m_parent;
}

bool ViewState::begin_draw()
{
    if ( !visible )
        return false;

    m_content_region = ImGuiEx::GetContentRegion();
    m_window_pos     = ImGui::GetWindowPos();

    if ( m_parent == nullptr)
    {
        m_box.set_size(m_content_region.size() );
        m_box.set_pos(m_content_region.center() ); // do not replace by this->set_pos(...)
    }

#if DEBUG_DRAW
    Rect r = get_rect(SCREEN_SPACE);
    if ( r.size().lensqr() < 0.1f )
    {
        r = m_content_region;
    }
    ImGuiEx::DebugRect(r.min, r.max, ImColor(255, 0, 0));     // box
    ImGuiEx::DebugLine(r.top_left(), r.bottom_right(), ImColor(255, 0, 0, 127));    // diagonal 1
    ImGuiEx::DebugLine(r.bottom_left(), r.top_right(), ImColor(255, 0, 0, 127 ));    // diagonal 2
    ImGuiEx::DebugCircle(r.center(), 2.f, ImColor(255, 0,0)); // center

    // center to parent center
    if ( m_parent != nullptr)
         ImGuiEx::DebugLine(m_parent->get_pos(SCREEN_SPACE), r.center(), ImColor(255, 0,255, 127 ), 4.f);
#endif
    return true;
}

Rect ViewState::get_content_region(Space space) const
{
    switch ( space )
    {
        case LOCAL_SPACE:
            return {
                Vec2{},
                m_content_region.max - m_content_region.min
            };
        case PARENT_SPACE:
            return m_content_region;
        case SCREEN_SPACE:
            return {
                m_window_pos + m_content_region.min,
                m_window_pos + m_content_region.max
            };
    }
}

void ViewState::add_child(ViewState* view)
{
    m_children.push_back(view);
    view->m_parent = this;
}
