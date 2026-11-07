#pragma once

#include "bdc/Array.hpp"
#include "Vec2.h"
#include "Rect.h"

namespace ndbl
{
    struct Bezier_Curve_Segment_2D
    {
        Vec2 p1; // point
        Vec2 p2; // control point
        Vec2 p3; // control point
        Vec2 p4; // point
    };

    void                        beziercurve_translate(Bezier_Curve_Segment_2D&, const Vec2& /* offset */);
    Rect                        beziercurve_bbox(const Bezier_Curve_Segment_2D&); //........................ fast and approx. bounding box
    bdc::Resizable_Array<Vec2>& beziercurve_tesselate(bdc::Resizable_Array<Vec2>& path, const Bezier_Curve_Segment_2D& curve, int num_segments = 0, float curve_tesselation_tol = 0.1f); // Copied from ImGui's internal and adapted
}