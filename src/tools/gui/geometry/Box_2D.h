#pragma once
#include "Spatial_Node.h"
#include "Rect.h"

namespace tools
{
    //
    // Box_2D wraps a Spatial_Node and a size (width/height)
    // Like Rect, Box_2D's origin is top_left corner.
    //
    struct Box_2D
    {
        Box_2D() = default;
        Box_2D(const Vec2& size);
        explicit Box_2D(const Rect& rect);

        Spatial_Node    spatial_node;
        Vec2            size;

        Vec2            position(Space space = PARENT_SPACE) const;
        void            set_position(Vec2 p, Space space = PARENT_SPACE ) { spatialnode_set_position(&spatial_node, p, space); }
        void            set_size(const Vec2& s);
        Rect            rect(Space = PARENT_SPACE) const;
        Vec2            pivot_position(const Vec2& pivot, Space = PARENT_SPACE) const;
    };

    void box2d_draw_debug_info(const Box_2D*);

    constexpr static Vec2 AXIS_MASK_X  = {1.f, 0.f};
    constexpr static Vec2 AXIS_MASK_Y  = {0.f, 1.f};
    constexpr static Vec2 AXIS_MASK_XY = AXIS_MASK_X + AXIS_MASK_Y;

    // Return the delta between two Box_2D pivots on a given axis
    Vec2 box2d_distance(
        const Box_2D&   leader  , const Vec2& leader_pivot,
        const Box_2D&   follower, const Vec2& follower_pivot,
        const Vec2&     axis_mask = AXIS_MASK_XY);
}