#pragma once
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "types.h"
#include <cassert>
#include <glm/glm/common.hpp>
#include <glm/glm/vec2.hpp>
#include <glm/glm/vec4.hpp>

namespace fw
{
    class Vec2
    {
    public:
        float x{};
        float y{};

        constexpr Vec2() = default;
        constexpr Vec2(float x, float y): x(x), y(y) {}
        constexpr Vec2(const Vec2& v): Vec2(v.x, v.y) {}
        constexpr Vec2(const glm::vec2& v): Vec2(v.x, v.y) {}
        explicit constexpr Vec2(float scalar): Vec2(scalar, scalar) {}
        constexpr Vec2(const ImVec2& v) : Vec2(v.x, v.y) {}

        operator ImVec2() const
        { return {x, y}; }

        operator glm::vec2() const
        { return {x, y}; }

#define UNARY_OPERATOR(_OP_)                \
    Vec2& operator _OP_ (const Vec2 & other)\
        {\
            x _OP_ other.x;\
            y _OP_ other.y;\
            return *this;\
        }

        UNARY_OPERATOR(+=)
        UNARY_OPERATOR(*=)
        UNARY_OPERATOR(-=)
        UNARY_OPERATOR(/=)

#define BINARY_OPERATOR(_OP_)                    \
    Vec2 operator _OP_ (const Vec2& other) const\
        { return { x _OP_ other.x, y _OP_ other.y}; }

        BINARY_OPERATOR(+)
        BINARY_OPERATOR(*)
        BINARY_OPERATOR(-)
        BINARY_OPERATOR(/)

        static Vec2 scale( Vec2 v, float magnitude )
        {
            v.x *= magnitude;
            v.y *= magnitude;
            return v;
        }

        static Vec2 round( Vec2 _vec)
        {
            return glm::round((glm::vec2)_vec);
        }
    };

    class Vec4
    {
    public:
        float x{};
        float y{};
        float z{};
        float w{};

        constexpr Vec4() = default;
        constexpr Vec4(float x, float y, float z, float w = 0.0f)
        : x(x), y(y), z(z), w(w)
        {}
        constexpr Vec4(const glm::vec4& vec): Vec4(vec.x, vec.y, vec.z, vec.w) {}

        template<typename VecT>
        constexpr explicit Vec4(const VecT& _vec)
        : Vec4(_vec.x, _vec.y, _vec.z, _vec.w)
        {}

        operator glm::vec4() const
        { return {x, y, z, w}; }

        operator ImVec4() const
        { return {x, y, z, w}; }
    };

    class Color : public Vec4
    {
    public:
        Color(u8_t r, u8_t g, u8_t b, u8_t a = 255)
        : Vec4(float(r)/255.f, float(g)/255.f, float(b)/255.f, float(a)/255.f)
        {}
    };

    class Rect
    {
    public:
        fw::Vec2 min;
        fw::Vec2 max;

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

        Rect(const ImRect& _rect)
        : Rect(fw::Vec2{_rect.Min}, fw::Vec2{_rect.Max} )
        {}

        template<typename RectT>
        Rect& operator=(const RectT& _rect)
        { *this = Rect{_rect}; return *this; }

        float height() const
        { return max.y - min.y; }

        float width() const
        { return max.x - min.x; }

        Vec2 center()
        { return { min.x + width() / 2.0f, min.y + height() / 2.0f }; }

        Vec2 tl() const // Top-Left corner
        { return min; }

        Vec2 bl() const // Bottom-Left corner
        { return { min.x, max.y }; }

        Vec2 br() const // Bottom-Right corner
        { return max; }

        Vec2 tr() const // Top-Right corner
        { return { max.x, min.y }; }

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

        bool contains( Rect other )
        {
            return min.x <= other.min.x && min.y <= other.min.y
                && max.x >= other.max.x && max.y >= other.max.y;
        }

        void expand_to_include( Rect other )
        {
            min.x = glm::min( min.x, other.min.x );
            max.x = glm::max( max.x, other.max.x );
            min.y = glm::min( min.y, other.min.y );
            max.y = glm::max( max.y, other.max.y );
        }
    };

    static float normalize(float _value, float _min, float _max)
    { return glm::clamp(_value, _min, _max) / (_max - _min); }

    /**
     * Interpolate linearly _source to _target with a _factor (in [0.0f, 1.0f] )
     */
    inline static float lerp(float _source, float _target, float _factor)
    { return glm::mix(_source, _target, glm::clamp(_factor, 0.0f, 1.0f)); }

    inline static Vec2 lerp( Vec2 _source, Vec2 _target, float _factor)
    { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

    inline static Vec4 lerp( Vec4 _source, Vec4 _target, float _factor)
    { return glm::mix((glm::vec4)_source, (glm::vec4)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

    static i64_t signed_diff(u64_t _left, u64_t _right)
    {
        bool left_greater_than_right = _left > _right;
        u64_t abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
        assert( abs_diff <= std::numeric_limits<u64_t>::max() );
        return left_greater_than_right ? (i64_t)abs_diff : -(i64_t)abs_diff;
    }
}
