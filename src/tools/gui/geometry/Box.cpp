#include "Box.h"
#include "tools/gui/ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define TOOLS_BOX_DEBUG 1
#endif

using namespace tools;

Box::Box(const Rect &r)
: xform()
{
    xform.set_pos(r.center());
    set_size(r.size());
}

Box::Box(
        XForm2D _xform,
        const Vec2 &_size
)
: xform(_xform)
{
    set_size(_size);
}

Vec2 Box::pivot(const Vec2& pivot, Space space) const
{
    return xform.get_pos(space) + _half_size * pivot;
}

Box Box::transform(const Box &box, const glm::mat3 &mat)
{
    ASSERT(false) // do we need the code below ??

    Box result = box;
    // Translate box position (box shape is relative to it)
    result.xform.set_pos(Vec2::transform(box.xform.get_pos(), mat));
    return result;
}

Rect Box::get_rect(Space space) const
{
    return {pivot(TOP_LEFT, space), pivot(BOTTOM_RIGHT, space)};
}

Vec2 Box::size() const
{
    return _half_size * 2.0f;
}

void Box::set_size(const Vec2& size)
{
    ASSERT(size.x >= 0) // Area cannot be zero
    ASSERT(size.y >= 0) //
    _half_size = size * 0.5f;
}

Vec2 Box::diff(
    const Box & leader,
    const Vec2& leader_pivot,
    const Box & follower,
    const Vec2& follower_pivot,
    const Vec2& axis
    )
{
    Vec2 follower_pos = follower.pivot(follower_pivot);
    Vec2 leader_pos   = leader.pivot(leader_pivot);
    Vec2 delta        = (leader_pos - follower_pos) * axis;

#if TOOLS_BOX_DEBUG
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
