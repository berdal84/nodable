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

        inline float height() const
        { return max.y - min.y; }

        inline float width() const
        { return max.x - min.x; }

        inline Vec2 center() const
        { return min + size() / 2.f; }

        inline Vec2 top_left() const
        { return min; }

        inline Vec2 bottom_left() const
        { return { min.x, max.y }; }

        inline Vec2 bottom_right() const
        { return max; }

        inline Vec2 top_right() const
        { return { max.x, min.y }; }

        inline Vec2 left() const
        { return { min.x, center().y }; }

        inline Vec2 right() const
        { return { max.x, center().y }; }

        inline Vec2 size() const
        { return { width(), height()}; }

        inline Rect&  translate_x( float d )
        {
            min.x += d;
            max.x += d;
            return *this;
        }

        inline Rect&  translate_y( float d )
        {
            min.y += d;
            max.y += d;
            return *this;
        }

        inline Rect& translate(const Vec2& _delta )
        {
            min += _delta;
            max += _delta;
            return *this;
        }

        inline Rect& expand(const Vec2&  offset) // Expand rectangle on both x and y axis
        {
            min -= offset;
            max += offset;
            return *this;
        }

        inline bool is_inverted() const
        { return min.x > max.x || min.y > max.y; }

        inline bool has_area() const
        { return width() != 0.f && height() != 0.f; }

        static Rect normalize(const Rect& _rect)
        {
            Rect result = _rect;
            if (result.min.x > result.max.x ) std::swap(result.min.x, result.max.x);
            if (result.min.y > result.max.y ) std::swap(result.min.y, result.max.y);
            return result;
        }
        
        static bool contains(const Rect& a, const Rect& b );

        static bool contains(const Rect& rect, const Vec2& point );

        static Rect merge(const Rect& a, const Rect& b );

        static Rect bbox(std::vector<Rect> rects );

        static Rect bbox(std::vector<Vec2> points );

        static Rect transform(const Rect &rect, const glm::mat3 &mat);

    };

}