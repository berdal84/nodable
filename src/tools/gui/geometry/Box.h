#pragma once
#include "XForm2D.h"

namespace tools
{
    class Box
    {
    public:
        Box() {}
        Box(const Rect&);
        Box(XForm2D, const Vec2& size);

        XForm2D       xform;
        Vec2          _half_size = {};

        void          set_size(const Vec2& s);
        Vec2          size() const;
        Rect          get_rect(Space = PARENT_SPACE) const;
        Vec2          pivot(const Vec2& pivot, Space = PARENT_SPACE) const;

        static Box    transform(const Box& box, const glm::mat3& mat);
        static Vec2   diff(const Box&  leader,   const Vec2& leader_pivot,
                           const Box&  follower, const Vec2& follower_pivot,
                           const Vec2& axis = XY_AXIS
                           ); // Return the delta between two Box pivots on a given axis
    };
}