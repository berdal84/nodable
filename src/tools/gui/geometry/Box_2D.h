#pragma once
#include "Spatial_Node.h"
#include "Rect.h"
#include "Axis.h"

namespace tools
{
    struct Box_2D
    {
        Box_2D() = default;
        Box_2D(const Vec2& size);
        explicit Box_2D(const Rect& rect); // will produce a centered box from rectangle size

        Spatial_Node    spatial_node;
        Vec2            _half_size;

        Vec2            position(Space space = PARENT_SPACE) const;
        void            set_position(Vec2 p, Space space = PARENT_SPACE ) { spatialnode_set_position(&spatial_node, p, space); }
        void            set_size(const Vec2& s);
        inline Vec2     size() const { return _half_size * 2.0f; }
        Rect            rect(Space = PARENT_SPACE) const;
        Vec2            pivot(const Vec2& pivot, Space = PARENT_SPACE) const;
    };

    void box2d_draw_debug_info(const Box_2D*);
    // Return the delta between two Box2D pivots on a given axis
    Vec2 box2d_diff(const Box_2D&  leader, const Vec2& leader_pivot,
                    const Box_2D&  follower, const Vec2& follower_pivot,
                    const Vec2& axis = AXIS_XY);
}