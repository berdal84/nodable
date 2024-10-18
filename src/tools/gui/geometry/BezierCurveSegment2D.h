#pragma once
#include "Vec2.h"
#include "Rect.h"
#include "tools/core/assertions.h"
#include <vector>

namespace tools
{
    struct BezierCurveSegment2D
    {
        Vec2 p1{}; // point
        Vec2 p2{}; // control point
        Vec2 p3{}; // control point
        Vec2 p4{}; // point

        BezierCurveSegment2D() = default;
        BezierCurveSegment2D(const Vec2& p1, const Vec2& p2, const Vec2& p3, const Vec2& p4)
        : p1(p1)
        , p2(p2)
        , p3(p3)
        , p4(p4)
        {}
        BezierCurveSegment2D(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
        : p1(x1, y1)
        , p2(x2, y2)
        , p3(x3, y3)
        , p4(x4, y4)
        {}

        void translate(const Vec2& offset)
        {
            p1 += offset;
            p2 += offset;
            p3 += offset;
            p4 += offset;
        }


        static Rect bbox(const BezierCurveSegment2D& segment);
        // Copied from ImGui's internal and adapted
        static std::vector<Vec2>* tesselate(std::vector<Vec2>* path, const BezierCurveSegment2D& curve, int num_segments = 0, float curve_tesselation_tol = 0.1f);
    };
}