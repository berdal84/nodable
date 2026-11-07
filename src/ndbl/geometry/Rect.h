#pragma once

#include "../Asserts.h"
#include "Vec2.h"

namespace ndbl
{
    struct Rect
    {
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
            : Rect( Vec2{_rect.Min}, Vec2{_rect.Max} )
        {}

        template<typename RectT>
        Rect& operator=(const RectT& _rect)
        { *this = Rect{_rect}; return *this; }
    };

    inline float rect_height(const Rect& rect)
    {
        return rect.max.y - rect.min.y;
    }

    inline float rect_width(const Rect& rect)
    {
        return rect.max.x - rect.min.x;
    }

    inline Vec2 rect_size(const Rect& rect)
    {
        return { rect_width(rect), rect_height(rect) };
    }

    inline Vec2 rect_center(const Rect& rect)
    {
        return rect.min + (rect.max - rect.min) / 2.f;
    }

    inline Vec2 rect_top_left(const Rect& rect)
    {
        return rect.min;
    }

    inline Vec2 rect_bottom_left(const Rect& rect)
    {
        return { rect.min.x, rect.max.y };
    }

    inline Vec2 rect_bottom_right(const Rect& rect)
    {
        return rect.max;
    }

    inline Vec2 rect_top_right(const Rect& rect)
    {
        return { rect.max.x, rect.min.y };
    }

    inline Vec2 rect_left(const Rect& rect)
    {
        return { rect.min.x, rect.min.y + (rect.max.y - rect.min.y) / 2.f };
    }

    inline Vec2 rect_right(const Rect& rect)
    {
        return { rect.max.x, rect.min.y + (rect.max.y - rect.min.y) / 2.f };
    }


    inline Rect& rect_translate_x(Rect& rect, float dx)
    {
        rect.min.x += dx;
        rect.max.x += dx;
        return rect;
    }

    inline Rect& rect_translate_y(Rect& rect, float dy)
    {
        rect.min.y += dy;
        rect.max.y += dy;
        return rect;
    }

    inline Rect& rect_translate(Rect& rect, const Vec2& delta )
    {
        rect.min += delta;
        rect.max += delta;
        return rect;
    }

    inline Rect& rect_expand(Rect& rect, float offset)
    {
        rect.min.x -= offset;
        rect.min.y -= offset;
        rect.max.x += offset;
        rect.max.y += offset;

        return rect;
    }

    inline Rect& rect_expand(Rect& rect, float x_offset, float y_offset)
    {
        rect.min.x -= x_offset;
        rect.min.y -= y_offset;
        rect.max.x += x_offset;
        rect.max.y += y_offset;

        return rect;
    }

    inline Rect& rect_expand(Rect& rect, float left, float top, float right, float bottom)
    {
        rect.min.x -= left;
        rect.min.y -= top;
        rect.max.x += right;
        rect.max.y += bottom;

        return rect;
    }

    inline bool rect_is_normalized(const Rect& rect)
    {
        return rect.min.x <= rect.max.x
            && rect.min.y <= rect.max.y;
    }

    inline bool rect_has_null_area(const Rect& rect)
    {
        return rect_width(rect)  == 0.f
            || rect_height(rect) == 0.f;
    }

    inline Rect rect_normalize(const Rect& rect) // swaps min and max if necessary to make sure min is actually the min for both axis
    {
        if(rect_is_normalized(rect))
        {
            return rect;
        }

        Rect result = rect;
        if (result.min.x > result.max.x ) std::swap(result.min.x, result.max.x);
        if (result.min.y > result.max.y ) std::swap(result.min.y, result.max.y);
        return result;
    }
    
    bool                 rect_contains(const Rect& a, const Rect& b );
    bool                 rect_contains_point(const Rect& rect, const Vec2& point );
    Rect                 rect_bounding_rect(const Rect& a, const Rect& b ); //...... returns the bbox of the two rectangles. If one of the rectangles has a null area the function simply returns the other rect.
    Rect                 rect_bounding_rect(const bdc::Array<Rect>& rect );
    Rect                 rect_bounding_rect(const bdc::Array<Vec2>& points );
}