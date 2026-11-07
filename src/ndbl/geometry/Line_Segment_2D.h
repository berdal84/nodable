#pragma once
#include "Vec2.h"

namespace ndbl
{
    struct Line_Segment_2D
    {
        Vec2 p1;
        Vec2 p2;
    };

    inline float line_sqrlen(const Line_Segment_2D& segment)
    {
        return vec2_sqrlen( segment.p2 - segment.p1 );
    }

    inline float line_point_distance_min(const Line_Segment_2D& segment, const Vec2& point )
    {
        // Adapted from https://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment

        // Return minimum distance between line segment vw and point p
        const float sqrlen = line_sqrlen(segment);  // i.e. |w-v|^2 -  avoid a sqrt
        if ( sqrlen == 0.0) return vec2_distance( point, segment.p1 );   // v == w case
        // Consider the line extending the segment, parameterized as v + t (w - v).
        // We find projection of point p onto the line.
        // It falls where t = [(p-v) . (w-v)] / |w-v|^2
        // We clamp t from [0,1] to handle points outside the segment vw.
        const float t = glm::max(0.f, glm::min(1.f, vec2_dot( point - segment.p1, segment.p2 - segment.p1 ) / sqrlen ));
        const Vec2 projection = segment.p1 + Vec2(t) * ( segment.p2 - segment.p1 );  // Projection falls on the segment
        return vec2_distance( point, projection );
    }
}