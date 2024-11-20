#pragma once
#include "SpatialNode2D.h"
#include "imgui.h"
#include "Rect.h"
#include "Axis.h"

namespace tools
{
    class BoxShape2D
    {
    public:
        BoxShape2D() {}
        BoxShape2D(const Rect&);
        BoxShape2D(SpatialNode2D, const Vec2& size);

        SpatialNode2D& spatial_node() { return _spatial_node; }
        const SpatialNode2D& spatial_node() const { return _spatial_node; }
        inline void   set_position(Vec2 p) { set_position(p, PARENT_SPACE); }
        inline void   set_position(Vec2 p, Space space) { _spatial_node.set_position(p, space); }
        void          set_size(const Vec2& s);
        Vec2          size() const { return _half_size * 2.0f; }
        const Vec2&   half_size() const { return _half_size; }
        Rect          rect(Space = PARENT_SPACE) const;
        Vec2          pivot(const Vec2& pivot, Space = PARENT_SPACE) const;
        void          draw_debug_info();
        static Vec2   diff(const BoxShape2D&  leader, const Vec2& leader_pivot,
                           const BoxShape2D&  follower, const Vec2& follower_pivot,
                           const Vec2& axis = XY_AXIS
                           ); // Return the delta between two Box pivots on a given axis
    private:
        Vec2          _half_size = {};
        SpatialNode2D _spatial_node;
    };
}