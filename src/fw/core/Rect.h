#pragma once
#include "Vec2.h"
#include "imgui/imgui_internal.h" // to provide ImRect conversion

namespace fw
{
    class Rect
    {
    public:
        Vec2 min;
        Vec2 max;

        Rect() = default;

        Rect(Vec2 min, Vec2 max)
        : min(min)
        , max(max)
        {}

        Rect(float w, float h)
        : min()
        , max(w, h)
        {}

        Rect(const Rect& _rect)
        : min(_rect.min)
        , max(_rect.max)
        {}

        template<typename RectT>
        Rect(const RectT& _rect)
            : Rect(fw::Vec2{_rect.Min}, fw::Vec2{_rect.Max} )
        {}

        template<typename RectT>
        Rect& operator=(const RectT& _rect)
        { *this = Rect{_rect}; return *this; }

        float height() const
        { return max.y - min.y; }

        float width() const
        { return max.x - min.x; }

        Vec2 center() const
        { return { min.x + width() / 2.0f, min.y + height() / 2.0f }; }

        Vec2 tl() const // Top-Left corner
        { return min; }

        Vec2 bl() const // Bottom-Left corner
        { return { min.x, max.y }; }

        Vec2 br() const // Bottom-Right corner
        { return max; }

        Vec2 tr() const // Top-Right corner
        { return { max.x, min.y }; }

        Vec2 left() const
        { return center() - Vec2{ width() / 2.f, 0.f }; }

        Vec2 size() const
        { return { width(), height()}; }

        void translate_x( float d )
        {
            min.x += d;
            max.x += d;
        }

        void translate_y( float d )
        {
            min.y += d;
            max.y += d;
        }

        void translate( Vec2 _delta )
        {
            min += _delta;
            max += _delta;
        }

        operator ImRect() const
        { return { min, max }; }

        void expand( Vec2 offset) // Expand rectangle on both x and y axis
        {
            min -= offset;
            max += offset;
        }

        static bool contains(const Rect& a, const Rect& b )
        {
            return a.min.x <= b.min.x && a.min.y <= b.min.y
                && a.max.x >= b.max.x && a.max.y >= b.max.y;
        }

        static Rect bbox(const Rect& a, const Rect& b ) // Return a rectangle overlapping the two rectangles
        {
            return {
                 {glm::min( a.min.x, b.min.x ), glm::min( a.min.y, b.min.y )},
                 {glm::max( a.max.x, b.max.x ), glm::max( a.max.y, b.max.y )}
            };
        }

        static Rect bbox( std::vector<Rect> rects ) // Return a rectangle overlapping all the rectangles.
        {
            if( rects.empty() )
            {
                return {};
            }
            Rect result = rects[0];
            for(auto it = rects.begin() +1; it != rects.end(); it++ )
            {
                result = Rect::bbox( result, *it );
            }
            return result;
        }
    };

}