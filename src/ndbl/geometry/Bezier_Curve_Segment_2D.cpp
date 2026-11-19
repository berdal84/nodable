#include "Bezier_Curve_Segment_2D.h"

#include "../Asserts.h"
#include "bdc/Array.hpp"

namespace ndbl
{

void beziercurve_translate(Bezier_Curve_Segment_2D& curve, const Vec2& offset)
{
    curve.p1 += offset;
    curve.p2 += offset;
    curve.p3 += offset;
    curve.p4 += offset;
}

static void PathBezierCubicCurveToCasteljau(bdc::Resizable_Array<Vec2>& path, const Bezier_Curve_Segment_2D& curv, float tess_tol, int level )
{
    float dx = curv.p4.x - curv.p1.x;
    float dy = curv.p4.y - curv.p1.y;
    float d2 = (curv.p2.x - curv.p4.x) * dy - (curv.p2.y - curv.p4.y) * dx;
    float d3 = (curv.p3.x - curv.p4.x) * dy - (curv.p3.y - curv.p4.y) * dx;
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2 + d3) * (d2 + d3) < tess_tol * (dx * dx + dy * dy))
    {
        array_append(path, Vec2{curv.p4.x, curv.p4.y});
    }
    else if (level < 10)
    {
        float x12 = (curv.p1.x + curv.p2.x) * 0.5f, y12 = (curv.p1.y + curv.p2.y) * 0.5f;
        float x23 = (curv.p2.x + curv.p3.x) * 0.5f, y23 = (curv.p2.y + curv.p3.y) * 0.5f;
        float x34 = (curv.p3.x + curv.p4.x) * 0.5f, y34 = (curv.p3.y + curv.p4.y) * 0.5f;
        float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
        float x234 = (x23 + x34) * 0.5f, y234 = (y23 + y34) * 0.5f;
        float x1234 = (x123 + x234) * 0.5f, y1234 = (y123 + y234) * 0.5f;
        PathBezierCubicCurveToCasteljau(path, Bezier_Curve_Segment_2D{ .p1 = curv.p1         , .p2 = { x12 , y12 } , .p3 = { x123, y123 }, .p4 = { x1234, y1234 }}, tess_tol, level + 1);
        PathBezierCubicCurveToCasteljau(path, Bezier_Curve_Segment_2D{ .p1 = { x1234, y1234 }, .p2 = { x234, y234 }, .p3 = { x34 , y34  }, .p4 = curv.p4         }, tess_tol, level + 1);
    }
}

static Vec2 BezierCubicCalc(const Bezier_Curve_Segment_2D& curve, float t )
{
    const float u = 1.0f - t;
    const float w1 = u * u * u;
    const float w2 = 3 * u * u * t;
    const float w3 = 3 * u * t * t;
    const float w4 = t * t * t;
    return {
        w1 * curve.p1.x + w2 * curve.p2.x + w3 * curve.p3.x + w4 * curve.p4.x,
        w1 * curve.p1.y + w2 * curve.p2.y + w3 * curve.p3.y + w4 * curve.p4.y
    };
}

bdc::Resizable_Array<Vec2>& beziercurve_tesselate(bdc::Resizable_Array<Vec2>& path, const Bezier_Curve_Segment_2D& curve, int num_segments, float curve_tesselation_tol )
{
    array_append(path, curve.p1);
    if (num_segments == 0)
    {
        NDBL_ASSERT(curve_tesselation_tol > 0.0f);
        PathBezierCubicCurveToCasteljau(path, curve, curve_tesselation_tol, 0); // 0 means Auto-tessellated
    }
    else
    {
        const float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            array_append(path, BezierCubicCalc(curve, t_step * (float)i_step));
        }
    }
    return path;
}

Rect beziercurve_bbox(const Bezier_Curve_Segment_2D& curve )
{
    bdc::Inlined_Array<Vec2, 4> points = { curve.p1, curve.p2, curve.p3, curve.p4 };
    Rect result = rect_bounding_rect(points);
    return result;
}

} // namespace ndbl