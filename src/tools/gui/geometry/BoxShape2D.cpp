#include "BoxShape2D.h"
#include "tools/gui/ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_draw_debug_info 1
#define DEBUG_diff            0
#endif

using namespace tools;

BoxShape2D::BoxShape2D(const Rect &r)
: xform()
{
    xform.set_pos(r.center());
    set_size(r.size());
}

BoxShape2D::BoxShape2D(
        SpatialNode2D _xform,
        const Vec2 &_size
)
: xform(_xform)
{
    set_size(_size);
}

Vec2 BoxShape2D::pivot(const Vec2& pivot, Space space) const
{
    if ( space == LOCAL_SPACE )
        return _half_size * pivot;
    return xform.get_pos(space) + _half_size * pivot;
}

Rect BoxShape2D::get_rect(Space space) const
{
    return {pivot(TOP_LEFT, space), pivot(BOTTOM_RIGHT, space)};
}

Vec2 BoxShape2D::size() const
{
    return _half_size * 2.0f;
}

void BoxShape2D::set_size(const Vec2& size)
{
    ASSERT(size.x >= 0); // Area cannot be zero
    ASSERT(size.y >= 0); //
    _half_size = size * 0.5f;
}

Vec2 BoxShape2D::diff(
    const BoxShape2D & leader,
    const Vec2& leader_pivot,
    const BoxShape2D & follower,
    const Vec2& follower_pivot,
    const Vec2& axis
    )
{
    Vec2 follower_pos = follower.pivot(follower_pivot, WORLD_SPACE);
    Vec2 leader_pos   = leader.pivot(leader_pivot, WORLD_SPACE);
    Vec2 delta        = (leader_pos - follower_pos) * axis;

#if DEBUG_diff
    ImColor color = ImColor(255, 0, 0, 127); // red to symbolize the constraint
    // Draw the constraint as a line
    ImGuiEx::DebugLine(follower_pos, follower_pos + delta, color, 2.f);
    // dot at the destination
    ImGuiEx::DebugCircle(leader_pos, 2.f, color, 0, 4.f);
    // Boxes
    ImGuiEx::DebugRect(leader.get_rect().min, leader.get_rect().max, ImColor(255,0,0, 127) );
    ImGuiEx::DebugRect(follower.get_rect().min, follower.get_rect().max, ImColor(0,255,0, 127) );
#endif

    return delta;
}


void BoxShape2D::draw_debug_info()
{
#if DEBUG_draw_debug_info
    Rect r = get_rect(WORLD_SPACE);
    if ( r.size().lensqr() < 0.1f )
        return;

    ImGuiEx::DebugRect(r.min, r.max, ImColor(255, 0, 0));     // box
    ImGuiEx::DebugLine(r.top_left(), r.bottom_right(), ImColor(255, 0, 0, 127));  // diagonal 1
    ImGuiEx::DebugLine(r.bottom_left(), r.top_right(), ImColor(255, 0, 0, 127 )); // diagonal 2
    ImGuiEx::DebugCircle(r.center(), 2.f, ImColor(255, 0,0)); // center

    // center to parent center
    if ( xform.get_parent() != nullptr)
         ImGuiEx::DebugLine(xform.get_parent()->get_pos(WORLD_SPACE ), r.center(), ImColor(255, 0, 255, 127 ), 4.f);
#endif
}