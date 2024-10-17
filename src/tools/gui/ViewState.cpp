#include "ViewState.h"
#include "ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_DRAW 0
#endif

using namespace tools;

ViewState::ViewState()
: ViewState(100.f, 100.f)
{
}

ViewState::ViewState(float width, float height)
: hovered(false)
, visible(true)
, selected(false)
, box()
, _parent(nullptr)
{
    box.set_size({width, height});
}

bool ViewState::begin_draw()
{
    if ( !visible )
        return false;

    _content_region = ImGuiEx::GetContentRegion();

    if (_parent == nullptr)
    {
        box.set_size(_content_region.size());
        box.xform.set_pos(_content_region.center()); // do not replace by this->set_pos(...)
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
        case PARENT_SPACE:
            return _content_region;
        case LOCAL_SPACE:
            return {
                Vec2{},
                _content_region.max - _content_region.min
            };
        case SCREEN_SPACE:
            return {
                _content_region.min,
                _content_region.max
            };
        default:
            ASSERT(false) // Not implemented yet
    }
}
