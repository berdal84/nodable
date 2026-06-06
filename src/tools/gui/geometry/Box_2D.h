#pragma once
#include "Spatial_Node.h"
#include "imgui.h"
#include "Rect.h"
#include "Axis.h"

namespace tools
{
    class Box_2D
    {
    public:
        Box_2D() = default;
        Box_2D(const Vec2& size);
        explicit Box_2D(const Rect& rect); // will produce a centered box from rectangle size

        Spatial_Node* spatial_node() { return &_spatial_node; }
        const Spatial_Node* spatial_node() const { return &_spatial_node; }
        Vec2          position(Space space = PARENT_SPACE) const;
        void          set_position(Vec2 p, Space space = PARENT_SPACE ) { _spatial_node.set_position(p, space); }
        void          set_size(const Vec2& s);
        Vec2          size() const { return _half_size * 2.0f; }
        const Vec2&   half_size() const { return _half_size; }
        Rect          rect(Space = PARENT_SPACE) const;
        Vec2          pivot(const Vec2& pivot, Space = PARENT_SPACE) const;
        void          draw_debug_info();
        static Vec2   diff(const Box_2D&  leader, const Vec2& leader_pivot,
                           const Box_2D&  follower, const Vec2& follower_pivot,
                           const Vec2& axis = XY_AXIS
                           ); // Return the delta between two Box pivots on a given axis
    private:
        Vec2          _half_size    = {0.f, 0.f};
        Spatial_Node   _spatial_node;
    };
}