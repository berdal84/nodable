#pragma once
#include "Vec2.h"
#include <vector>

namespace tools
{
    class Rect
    {
    public:
        Vec2 min;
        Vec2 max;

        Rect() = default;

        Rect(const Vec2& min, const Vec2& max)
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
            : Rect( tools::Vec2{_rect.Min}, tools::Vec2{_rect.Max} )
        {}

        template<typename RectT>
        Rect& operator=(const RectT& _rect)
        { *this = Rect{_rect}; return *this; }

        float height() const
        { return max.y - min.y; }

        float width() const
        { return max.x - min.x; }

        Vec2 center() const
        { return min + size() / 2.f; }

        Vec2 tl() const // Top-Left corner
        { return min; }

        Vec2 bl() const // Bottom-Left corner
        { return { min.x, max.y }; }

        Vec2 br() const // Bottom-Right corner
        { return max; }

        Vec2 tr() const // Top-Right corner
        { return { max.x, min.y }; }

        Vec2 left() const
        { return { min.x, center().y }; }

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

        void translate(const Vec2& _delta )
        {
            min += _delta;
            max += _delta;
        }

        void expand(const Vec2&  offset) // Expand rectangle on both x and y axis
        {
            min -= offset;
            max += offset;
        }

        bool is_inverted() const
        { return min.x > max.x || min.y > max.y; }

        static Rect normalize(const Rect& _rect)
        {
            Rect result = _rect;
            if (result.min.x > result.max.x ) std::swap(result.min.x, result.max.x);
            if (result.min.y > result.max.y ) std::swap(result.min.y, result.max.y);
            return result;
        }
        
        static bool contains(const Rect& a, const Rect& b );

        static bool contains(const Rect& rect, const Vec2& point );

        static Rect bbox(const Rect& a, const Rect& b );

        static Rect bbox(std::vector<Rect> rects );

        static Rect bbox(std::vector<Vec2> points );

        static Rect transform(const Rect &rect, const glm::mat3 &mat);
    };

}