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
        fw::Vec2 Min;
        fw::Vec2 Max;

        Rect()
        : Min()
        , Max()
        {};

        Rect(fw::Vec2 min, fw::Vec2 max)
        : Min(min)
        , Max(max)
        {}

        Rect(float w, float h)
        : Min()
        , Max(w, h)
        {}

        Rect(const Rect& _rect)
        : Min(_rect.Min)
        , Max(_rect.Max)
        {}

        Rect(const ImRect& _rect)
        : Rect(fw::Vec2{_rect.Min}, fw::Vec2{_rect.Max} )
        {}

        template<typename RectT>
        Rect& operator=(const RectT& _rect)
        { *this = Rect{_rect}; return *this; }

        float GetHeight() const
        { return Max.y - Min.y; }

        float GetWidth() const
        { return Max.x - Min.x; }

        Vec2 get_center()
        { return { Min.x + GetWidth() / 2.0f, Min.y + GetHeight() / 2.0f }; }

        Vec2 get_TL() const
        { return Min; }

        Vec2 get_BL() const
        { return { Min.x, Max.y }; }

        Vec2 get_BR() const
        { return Max; }

        Vec2 get_TR() const
        { return { Max.x, Min.y }; }

        Vec2 get_size() const
        { return { GetWidth(), GetHeight()}; }

        void translate_x( float d )
        { Min.x += d; Max.x += d; }

        void translate_y( float d )
        { Min.y += d; Max.y += d; }

        void translate( Vec2 _delta )
        {
            Min += _delta;
            Max += _delta;
        }

        operator ImRect() const
        { return { Min, Max}; }

        void expand( Vec2 size)
        {
            Vec2 half_size = size * Vec2(0.5f);
            Min -= half_size;
            Max += half_size;
        }

        bool contains( Rect other )
        {
            return Min.x <= other.Min.x && Min.y <= other.Min.y
                && Max.x >= other.Max.x && Max.y >= other.Max.y;
        }

        void expand_to_include( Rect other )
        {
            Min.x = glm::min( Min.x, other.Min.x );
            Max.x = glm::max( Max.x, other.Max.x );
            Min.y = glm::min( Min.y, other.Min.y );
            Max.y = glm::max( Max.y, other.Max.y );
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
