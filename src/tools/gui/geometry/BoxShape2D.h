#pragma once
#include "SpatialNode2D.h"
#include "imgui.h"

namespace tools
{
    class BoxShape2D
    {
    public:
        BoxShape2D() {}
        BoxShape2D(const Rect&);
        BoxShape2D(SpatialNode2D, const Vec2& size);

        SpatialNode2D       xform;
        Vec2          _half_size = {};
        inline void   set_pos(Vec2 p) { set_pos(p, PARENT_SPACE); }
        inline void   set_pos(Vec2 p, Space space) { xform.set_pos(p, space); }
        void          set_size(const Vec2& s);
        Vec2          size() const;
        Rect          get_rect(Space = PARENT_SPACE) const;
        Vec2          pivot(const Vec2& pivot, Space = PARENT_SPACE) const;
        void          draw_debug_info();
        static Vec2   diff(const BoxShape2D&  leader, const Vec2& leader_pivot,
                           const BoxShape2D&  follower, const Vec2& follower_pivot,
                           const Vec2& axis = XY_AXIS
                           ); // Return the delta between two Box pivots on a given axis
    };
}