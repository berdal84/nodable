#include "Box_2D.h"
#include "core/Asserts.h"
#include "gui/geometry/Spatial_Node.h"
#include "tools/gui/ImGuiEx.h"

#ifdef TOOLS_DEBUG
#define DEBUG_draw_debug_info 1
#define DEBUG_diff            0
#endif

using namespace tools;

Box_2D::Box_2D(const Vec2& _size)
{
    set_size(_size);
}

Box_2D::Box_2D(const Rect& rect)
{
    set_size(rect.size());
    spatialnode_set_position(&spatial_node, rect.min);
}

Vec2 Box_2D::pivot_position(const Vec2& pivot, Space space) const
{
    if ( space == LOCAL_SPACE )
        return size * pivot;
    return position(space) + size * pivot;
}

Vec2 Box_2D::position(Space space) const
{
    return spatialnode_position(&spatial_node, space);
}

Rect Box_2D::rect(Space space) const
{
    Vec2 pos = spatialnode_position(&spatial_node, space);

    Rect r;
    r.min = pos;
    r.max = pos + size;
    return r;
}

void Box_2D::set_size(const Vec2& _size)
{
    ASSERT(_size.x >= 0); // Area cannot be zero
    ASSERT(_size.y >= 0); //
    size = _size;
}

Vec2 tools::box2d_distance(
    const Box_2D&   leader,
    const Vec2&     leader_pivot,
    const Box_2D&   follower,
    const Vec2&     follower_pivot,
    const Vec2&     axis_mask
    )
{
    Vec2 follower_pos = follower.pivot_position(follower_pivot, WORLD_SPACE);
    Vec2 leader_pos   = leader.pivot_position(leader_pivot, WORLD_SPACE);
    Vec2 delta        = (leader_pos - follower_pos) * axis_mask;

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


void tools::box2d_draw_debug_info(const Box_2D* box)
{
#if DEBUG_draw_debug_info
    Rect r = box->rect(WORLD_SPACE);
    if ( r.size().lensqr() < 0.1f )
        return;

    ImGuiEx::DebugRect(r.min, r.max, ImColor(255, 0, 0));     // box
    ImGuiEx::DebugLine(r.top_left(), r.bottom_right(), ImColor(255, 0, 0, 127));  // diagonal 1
    ImGuiEx::DebugLine(r.bottom_left(), r.top_right(), ImColor(255, 0, 0, 127 )); // diagonal 2
    ImGuiEx::DebugCircle(r.center(), 2.f, ImColor(255, 0,0)); // center

    // center to parent center
    if ( box->spatial_node.parent != nullptr)
    {
        ImGuiEx::DebugLine(spatialnode_position(box->spatial_node.parent, WORLD_SPACE), r.center(), ImColor(255, 0, 255, 127 ), 4.f);
    }
#endif
}
