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
    class vec2: public glm::vec2
    {
    public:
        using underlying_type = glm::vec2;
        constexpr vec2(): glm::vec2() {};
        constexpr vec2(float scalar): vec2(scalar, scalar) {}
        constexpr vec2(float x, float y): glm::vec2(x, y) {}
        constexpr vec2(const vec2& vec): glm::vec2(vec) {}
        constexpr vec2(const glm::vec2& vec): glm::vec2(vec) {}
        constexpr vec2(const ImVec2& _vec)
        : glm::vec2(_vec.x, _vec.y)
        {}

        operator ImVec2() const
        { return {x, y}; }

        static vec2 round(vec2 _vec)
        { return (vec2)glm::round((glm::vec2)_vec); }

        vec2& operator+=(const vec2& other)
        { underlying_type::operator+=((underlying_type)other); return *this; }

        vec2& operator*=(const vec2& other)
        { underlying_type::operator*=((underlying_type)other); return *this; }
    };

    class vec4: public glm::vec4
    {
    public:
        constexpr vec4(): glm::vec4() {};
        constexpr vec4(float x, float y, float z, float w = 0.0f)
        : glm::vec4(x, y, z, w)
        {}
        constexpr vec4(const glm::vec4& vec): glm::vec4(vec) {}

        template<typename VecT>
        constexpr explicit vec4(const VecT& _vec)
        : vec4(_vec.x, _vec.y, _vec.z, _vec.w)
        {}

        operator ImVec4() const
        { return {x, y, z, w}; }
    };

    class color : public vec4
    {
    public:
        color(u8_t r, u8_t g, u8_t b, u8_t a = 255)
        : vec4(float(r)/255.f, float(g)/255.f, float(b)/255.f, float(a)/255.f)
        {}
    };

    class rect
    {
    public:
        fw::vec2 Min;
        fw::vec2 Max;

        rect()
        : Min()
        , Max()
        {};

        rect(fw::vec2 min, fw::vec2 max)
        : Min(min)
        , Max(max)
        {}

        rect(float w, float h)
        : Min()
        , Max(w, h)
        {}

        rect(const rect& _rect)
        : Min(_rect.Min)
        , Max(_rect.Max)
        {}

        rect(const ImRect& _rect)
        : rect(fw::vec2{_rect.Min}, fw::vec2{_rect.Max} )
        {}

        template<typename RectT>
        rect& operator=(const RectT& _rect)
        { *this = rect{_rect}; return *this; }

        float GetHeight() const
        { return Max.y - Min.y; }

        float GetWidth() const
        { return Max.x - Min.x; }

        vec2 GetCenter()
        { return { Min.x + GetWidth() / 2.0f, Min.y + GetHeight() / 2.0f }; }

        vec2 GetTL() const
        { return Min; }

        vec2 GetBL() const
        { return { Min.x, Max.y }; }

        vec2 GetBR() const
        { return Max; }

        vec2 GetTR() const
        { return { Max.x, Min.y }; }

        vec2 GetSize() const
        { return { GetWidth(), GetHeight()}; }

        void TranslateX( float d )
        { Min.x += d; Max.x += d; }

        void TranslateY( float d )
        { Min.y += d; Max.y += d; }

        void Translate( vec2 _delta )
        {
            Min += _delta;
            Max += _delta;
        }

        operator ImRect() const
        { return { Min, Max}; }

        void Expand( vec2 size )
        { assert(false); /* TODO */ }
        bool Contains( rect rect1 )
        { assert(false); /* TODO */ }
    };

    vec2 magnitude(vec2 _vec, float _f)
    { return static_cast<vec2>( _vec * _f ); }

    static float normalize(float _value, float _min, float _max)
    { return glm::clamp(_value, _min, _max) / (_max - _min); }

    /**
     * Interpolate linearly _source to _target with a _factor (in [0.0f, 1.0f] )
     */
    inline static float lerp(float _source, float _target, float _factor)
    { return glm::mix(_source, _target, glm::clamp(_factor, 0.0f, 1.0f)); }

    inline static vec2 lerp(vec2 _source, vec2 _target, float _factor)
    { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

    inline static vec4 lerp(vec4 _source, vec4 _target, float _factor)
    { return glm::mix((glm::vec4)_source, (glm::vec4)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

    static i64_t signed_diff(u64_t _left, u64_t _right)
    {
        bool left_greater_than_right = _left > _right;
        u64_t abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
        assert( abs_diff <= std::numeric_limits<u64_t>::max() );
        return left_greater_than_right ? (i64_t)abs_diff : -(i64_t)abs_diff;
    }
}
