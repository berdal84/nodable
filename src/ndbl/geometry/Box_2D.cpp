#include "Box_2D.h"

#include "../ImGuiEx.h"
#include "Spatial_Node.h"

#ifdef NDBL_DEBUG
#define NDBL_DEBUG_draw_debug_info 1
#define NDBL_DEBUG_box2d_distance  0
#endif

namespace ndbl
{

void box2d_init(Box_2D& box)
{
    spatialnode_init(box.spatial_node);
}

void box2d_release(Box_2D& box)
{
    spatialnode_release(box.spatial_node);
}

Vec2 box2d_pivot_position(const Box_2D& box, const Vec2& pivot, Space space)
{
    if ( space == LOCAL_SPACE )
    {
        return box.size * pivot;
    }
    return box2d_position(box, space) + box.size * pivot;
}

Vec2 box2d_position(const Box_2D& box, Space space)
{
    return spatialnode_position(box.spatial_node, space);
}

void box2d_set_position(Box_2D& box, Vec2 p, Space space )
{
    spatialnode_set_position(box.spatial_node, p, space);
}

Rect box2d_to_rect(const Box_2D& box, Space space)
{
    Vec2 pos = spatialnode_position(box.spatial_node, space);

    Rect r;
    r.min = pos;
    r.max = pos + box.size;
    return r;
}

Vec2 box2d_distance(
    const Box_2D&   dest,
    const Vec2&     dest_pivot,
    const Box_2D&   src,
    const Vec2&     src_pivot,
    const Vec2&     axis_mask
    )
{
    const Vec2 src_pos  = box2d_pivot_position(src , src_pivot , WORLD_SPACE);
    const Vec2 dest_pos = box2d_pivot_position(dest, dest_pivot, WORLD_SPACE);
    const Vec2 delta    = (dest_pos - src_pos) * axis_mask;

#if NDBL_DEBUG_box2d_distance
    ImColor color = ImColor(255, 0, 0, 127); // red to symbolize the constraint
    // Draw the constraint as a line
    ImGuiEx::DebugLine(src_pos, src_pos + delta, color, 2.f);
    // dot at the destination
    ImGuiEx::DebugCircle(dest_pos, 2.f, color, 0, 4.f);
    // Boxes
    Rect dest_box = box2d_rect(dest);
    ImGuiEx::DebugRect(dest_box.min, dest_box.max, ImColor(255,0,0, 127) );
    Rect src_box = box2d_rect(src);
    ImGuiEx::DebugRect(src_box.min, src_box.max, ImColor(0,255,0, 127) );
#endif

    return delta;
}


void box2d_draw_debug_info(const Box_2D& box)
{
#if NDBL_DEBUG_draw_debug_info
    Rect r = box2d_to_rect(box, WORLD_SPACE);

    if ( vec2_sqrlen( rect_size(r) ) < 0.1f )
    {
        return;
    }

    ImGuiEx::DebugRect(r.min, r.max, ImColor(255, 0, 0));     // box
    ImGuiEx::DebugLine(rect_top_left(r), rect_bottom_right(r), ImColor(255, 0, 0, 127));  // diagonal 1
    ImGuiEx::DebugLine(rect_bottom_left(r), rect_top_right(r), ImColor(255, 0, 0, 127 )); // diagonal 2
    ImGuiEx::DebugCircle(rect_center(r), 2.f, ImColor(255, 0,0)); // center

    // center to parent center
    if ( box.spatial_node.parent != nullptr)
    {
        Vec2 p0 = spatialnode_position(*box.spatial_node.parent, WORLD_SPACE);
        Vec2 p1 = rect_center(r);
        ImGuiEx::DebugLine( p0, p1, ImColor(255, 0, 255, 127 ), 4.f);
    }
#endif
}

} // namespace ndbl