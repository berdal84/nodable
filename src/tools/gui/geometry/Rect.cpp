#include "Rect.h"
#include "tools/core/assertions.h"

using namespace tools;

Rect tools::Rect::bbox(const std::vector<Vec2>* points )
{
    ASSERT(points->empty() == false);
    Vec2 first = points->front();
    Rect result{ first, first };
    for ( auto it = points->begin() + 1; it != points->end(); it++ )
    {
        if      ( it->x < result.min.x ) result.min.x = it->x;
        else if ( it->x > result.max.x ) result.max.x = it->x;
        if      ( it->y < result.min.y ) result.min.y = it->y;
        else if ( it->y > result.max.y ) result.max.y = it->y;
    }
    return result;
}

Rect Rect::bbox(const std::vector<Rect>* rects )// Return a rectangle overlapping all the rectangles.
{
    if( rects->empty() )
        return {};
    if ( rects->size() == 1 )
        return rects->front();
    Rect result = rects->front();
    for(auto i = 1; i < rects->size(); ++i )
        result = Rect::merge(result, (*rects)[i] );
    return result;
}

Rect Rect::merge(const Rect& a, const Rect& b )// Return a rectangle overlapping the two rectangles
{
    ASSERT(!a.is_inverted());
    ASSERT(!b.is_inverted());
    Rect result;
    result.min.x = glm::min( a.min.x, b.min.x );
    result.min.y = glm::min( a.min.y, b.min.y );
    result.max.x = glm::max( a.max.x, b.max.x );
    result.max.y = glm::max( a.max.y, b.max.y );
    return result;
}

bool Rect::contains( const Rect& a, const Rect& b )
{
    ASSERT(!a.is_inverted());
    ASSERT(!b.is_inverted());

    if ( a.min.x <= b.min.x )
        if ( a.min.y <= b.min.y )
            if ( a.max.x >= b.max.x )
                if ( a.max.y >= b.max.y )
                    return true;
    return false;
}

bool Rect::contains( const Rect& rect, const Vec2& point )
{
    ASSERT(!rect.is_inverted());

    return point.x >= rect.min.x &&
           point.x <= rect.max.x &&
           point.y >= rect.min.y &&
           point.y <= rect.max.y;
}