#include "Box.h"
#include "tools/gui/ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define TOOLS_BOX_DEBUG 1
#endif

using namespace tools;

Box Box::align(
    const Box & leader,
    const Vec2& leader_pivot,
    const Box & follower,
    const Vec2& follower_pivot,
    const Vec2& axis
    )
{
    Vec2 follower_pos = follower.get_pivot(follower_pivot);
    Vec2 leader_pos   = leader.get_pivot(leader_pivot);
    Vec2 delta        = (leader_pos - follower_pos) * axis;

#if TOOLS_BOX_DEBUG
    ImColor color = ImColor(255, 0, 0, 127); // red to symbolize the constraint
    // Draw the constraint as a line
    ImGuiEx::DebugLine(follower_pos, follower_pos + delta, color, 2.f);
    // dot at the destination
    ImGuiEx::DebugCircle(leader_pos, 2.f, color, 0, 4.f);
#endif

    Box aligned_follower = follower;
    aligned_follower.translate( delta );
    return aligned_follower;
}

Vec2 Box::get_pivot(const Vec2 &pivot) const
{
    return m_xform.pos() + m_half_size * pivot;
}

Box Box::transform(const Box &box, const glm::mat3 &mat)
{
    Box result = box;
    // Translate box position (box shape is relative to it)
    result.set_pos( Vec2::transform( box.get_pos(), mat ) );
    return result;
}