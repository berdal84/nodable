#pragma once
#include "Spatial_Node.h"
#include "Rect.h"

namespace ndbl
{
    struct Box_2D
    {
        Vec2            size; 
        Spatial_Node    spatial_node; //....... origin, is top-left corner

        Box_2D() = default;
        Box_2D(Vec2 initial_size): size(initial_size), spatial_node() {}
    };

    void box2d_release(Box_2D&);
    Vec2 box2d_pivot_position(const Box_2D&, const Vec2& pivot, Space = PARENT_SPACE);
    Vec2 box2d_position(const Box_2D&, Space = PARENT_SPACE);
    void box2d_set_position(Box_2D&, Vec2 pos, Space = PARENT_SPACE);
    Rect box2d_to_rect(const Box_2D&, Space = PARENT_SPACE);
    void box2d_draw_debug_info(const Box_2D&);

    constexpr static Vec2 AXIS_MASK_X  = {1.f, 0.f};
    constexpr static Vec2 AXIS_MASK_Y  = {0.f, 1.f};
    constexpr static Vec2 AXIS_MASK_XY = AXIS_MASK_X + AXIS_MASK_Y;

    // Return the delta between two Box_2D pivots on a given axis
    Vec2 box2d_distance(
        const Box_2D&   dest, const Vec2& dest_pivot,
        const Box_2D&   src, const Vec2& src_pivot,
        const Vec2&     axis_mask = AXIS_MASK_XY);
}