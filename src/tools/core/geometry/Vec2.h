#pragma once

#include <glm/common.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec2.hpp>

#include "tools/core/types.h"

namespace tools
{
    struct Vec2
    {
        float x, y;

        constexpr Vec2(): x(0.f), y(0.f) {};
        constexpr Vec2(float x,float y): x(x), y(y) {}
        constexpr Vec2(const glm::vec2& v): Vec2( v.x, v.y ) {}
        constexpr Vec2(float scalar): Vec2( scalar, scalar ) {}

        operator glm::vec2() const
        { return { x, y }; }

        Vec2& operator +=( const Vec2 & other )
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        Vec2& operator -=( const Vec2 & other )
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        Vec2& operator *=( const Vec2 & other )
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        Vec2& operator /=( const Vec2 & other )
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        Vec2 operator +( const Vec2& other ) const
        { return { x + other.x, y + other.y }; }

        Vec2 operator -( const Vec2& other ) const
        { return { x - other.x, y - other.y }; }

        Vec2 operator *( const Vec2& other ) const
        { return { x * other.x, y * other.y }; }

        Vec2 operator /( const Vec2& other ) const
        { return { x / other.x, y / other.y }; }

        Vec2 operator +( float f ) const
        { return *this + Vec2(f); }

        Vec2 operator -( float f ) const
        { return *this - Vec2(f); }

        Vec2 operator -() const
        { return *this * Vec2(-1.f); }

        Vec2 operator /( float f ) const
        { return *this / Vec2(f); }

        Vec2 operator *( float f ) const
        { return *this * Vec2(f); }

        inline float lensqr() const
        { return Vec2::lensqr(*this); }

        inline void round()
        { *this = Vec2::round(*this); }

        inline static float lensqr(const Vec2& v)
        { return v.x*v.x+v.y*v.y; }

        inline static Vec2 scale(const Vec2& v, float magnitude )
        { return v * Vec2(magnitude); }

        inline static Vec2 round(const Vec2& v )
        { return glm::round( (glm::vec2) v ); }

        inline static Vec2 lerp( const Vec2& _source, const Vec2& _target, float _factor)
        { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

        inline static Vec2 transform(const Vec2& v, const glm::mat3& m)
        {
            auto result = m * glm::vec3(v.x, v.y, 1.f);
            return {result.x, result.y};
        }

        inline static float distance( const Vec2& a, const Vec2&  b )
        { return glm::distance((glm::vec2)a, (glm::vec2)b); }

        inline static float dot( const Vec2& a, const Vec2&  b )
        { return glm::dot((glm::vec2)a, (glm::vec2)b); }

        inline static Vec2 normalize(const Vec2& v)
        { return glm::normalize( (glm::vec2)v ); }
    };

    static_assert(std::is_constructible_v<Vec2>);
    static_assert(std::is_copy_constructible_v<Vec2>);
    static_assert(std::is_copy_assignable_v<Vec2>);
    static_assert(std::is_default_constructible_v<Vec2>);
    static_assert(std::is_trivially_copyable_v<Vec2>);
}