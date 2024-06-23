#pragma once
#include "Vec2.h"

namespace tools
{
    struct LineSegment
    {
        Vec2 p1;
        Vec2 p2;

        float sqrlen() const
        { return Vec2::lensqr( p2 - p1 ); }

        static float point_minimum_distance(const LineSegment& segment, const Vec2& point )
        {
            // Adapted from https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment

            // Return minimum distance between line segment vw and point p
            const float sqrlen = segment.sqrlen();  // i.e. |w-v|^2 -  avoid a sqrt
            if ( sqrlen == 0.0) return Vec2::distance( point, segment.p1 );   // v == w case
            // Consider the line extending the segment, parameterized as v + t (w - v).
            // We find projection of point p onto the line.
            // It falls where t = [(p-v) . (w-v)] / |w-v|^2
            // We clamp t from [0,1] to handle points outside the segment vw.
            const float t = glm::max(0.f, glm::min(1.f, Vec2::dot( point - segment.p1, segment.p2 - segment.p1 ) / sqrlen ));
            const Vec2 projection = segment.p1 + Vec2(t) * ( segment.p2 - segment.p1 );  // Projection falls on the segment
            return Vec2::distance( point, projection );
        }
    };
}