#include "Rect.h"

#include "../Asserts.h"

namespace ndbl
{

Rect rect_bounding_rect(const bdc::Array<Vec2>& points )
{
    NDBL_ASSERT(points.size != 0);
    Vec2 first = points[0];
    Rect result{ first, first };
    for ( auto it = points.begin() + 1; it != points.end(); it++ ) // Skip the first point since it is used to build the initial Rect.
    {
        if      ( it->x < result.min.x ) result.min.x = it->x;
        else if ( it->x > result.max.x ) result.max.x = it->x;
        
        if      ( it->y < result.min.y ) result.min.y = it->y;
        else if ( it->y > result.max.y ) result.max.y = it->y;
    }
    return result;
}

Rect rect_bounding_rect(const bdc::Array<Rect>& rectangles )
{
    Rect result;
    for(const Rect& rect : rectangles)
    {
        result = rect_bounding_rect(result, rect);
    }
    return result;
}

Rect rect_bounding_rect(const Rect& a, const Rect& b )
{
    NDBL_ASSERT(rect_is_normalized(a));
    NDBL_ASSERT(rect_is_normalized(b));

    if( rect_has_null_area(a) ) return b;
    if( rect_has_null_area(b) ) return a;

    Rect result;
    result.min.x = glm::min( a.min.x, b.min.x );
    result.min.y = glm::min( a.min.y, b.min.y );
    result.max.x = glm::max( a.max.x, b.max.x );
    result.max.y = glm::max( a.max.y, b.max.y );
    
    return result;
}

bool rect_contains( const Rect& a, const Rect& b )
{
    NDBL_ASSERT(rect_is_normalized(a));
    NDBL_ASSERT(rect_is_normalized(b));

    return a.min.x <= b.min.x
        && a.min.y <= b.min.y
        && a.max.x >= b.max.x
        && a.max.y >= b.max.y;
}

bool rect_contains_point( const Rect& rect, const Vec2& point )
{
    NDBL_ASSERT(rect_is_normalized(rect));

    return point.x >= rect.min.x &&
           point.x <= rect.max.x &&
           point.y >= rect.min.y &&
           point.y <= rect.max.y;
}

} // namespace ndbl