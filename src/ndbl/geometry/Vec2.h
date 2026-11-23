#pragma once

#include "glm/common.hpp"
#include "glm/ext/matrix_float3x3.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/vec2.hpp"

#include "bdc/Types.hpp"

namespace ndbl
{
    struct Vec2
    {
        float x, y;

        constexpr Vec2(): x(0.f), y(0.f) {};
        constexpr Vec2(int x, int y): x((float)x), y((float)y) {}
        constexpr Vec2(float x, float y): x(x), y(y) {}
        constexpr Vec2(const glm::vec2& v): Vec2( v.x, v.y ) {}
        constexpr Vec2(float scalar): Vec2( scalar, scalar ) {}

        operator glm::vec2() const
        { return { x, y }; }

        constexpr Vec2& operator +=( const Vec2 & other )
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vec2& operator -=( const Vec2 & other )
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr Vec2& operator *=( const Vec2 & other )
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        constexpr Vec2& operator /=( const Vec2 & other )
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

        constexpr Vec2 operator +( const Vec2& other ) const
        { return { x + other.x, y + other.y }; }

        constexpr Vec2 operator -( const Vec2& other ) const
        { return { x - other.x, y - other.y }; }

        constexpr Vec2 operator *( const Vec2& other ) const
        { return { x * other.x, y * other.y }; }

        constexpr Vec2 operator /( const Vec2& other ) const
        { return { x / other.x, y / other.y }; }

        constexpr Vec2 operator +( float f ) const
        { return *this + Vec2(f); }

        constexpr Vec2 operator -( float f ) const
        { return *this - Vec2(f); }

        constexpr Vec2 operator -() const
        { return *this * Vec2(-1.f); }

        constexpr Vec2 operator /( float f ) const
        { return *this / Vec2(f); }

        constexpr Vec2 operator *( float f ) const
        { return *this * Vec2(f); }

        bool operator==(const Vec2& other) const
        { return this->x == other.x && this->y == other.y; }
    };
    
    static_assert(std::is_constructible_v<Vec2>);
    static_assert(std::is_copy_constructible_v<Vec2>);
    static_assert(std::is_copy_assignable_v<Vec2>);
    static_assert(std::is_default_constructible_v<Vec2>);
    static_assert(std::is_trivially_copyable_v<Vec2>);

    inline constexpr float vec2_sqrlen(const Vec2& v)
    {
        return v.x*v.x + v.y*v.y;
    }

    inline constexpr Vec2 vec2_scale(const Vec2& v, float magnitude )
    {
        return v * Vec2(magnitude);
    }

    inline Vec2 vec2_round(const Vec2& v )
    {
        return glm::round( (glm::vec2) v );
    }

    inline Vec2 vec2_lerp( const Vec2& src, const Vec2& dest, float _factor)
    {
        return glm::mix((glm::vec2)src, (glm::vec2)dest, glm::clamp(_factor, 0.0f, 1.0f));
    }

    inline Vec2 vec2_transform(const Vec2& v, const glm::mat3& m)
    {
        glm::vec3 result = m * glm::vec3(v.x, v.y, 1.f);
        return {result.x, result.y};
    }

    inline float vec2_distance(const Vec2& p0, const Vec2&  p1) // distance p0-p1
    {
        return glm::distance((glm::vec2)p0, (glm::vec2)p1);
    }

    inline float vec2_dot(const Vec2& a, const Vec2&  b)
    {
        return glm::dot((glm::vec2)a, (glm::vec2)b);
    }

    inline Vec2 vec2_normalize(const Vec2& v)
    {
        return glm::normalize( (glm::vec2)v );
    }
}