#pragma once

#include "glm/glm/common.hpp"
#include "glm/glm/ext/matrix_float3x3.hpp"
#include "glm/glm/ext/matrix_transform.hpp"
#include "glm/glm/vec2.hpp"

#include "tools/core/types.h"

namespace tools
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

        constexpr  float lensqr() const
        { return Vec2::lensqr(*this); }

        void round()
        { *this = Vec2::round(*this); }

        constexpr static float lensqr(const Vec2& v)
        { return v.x*v.x+v.y*v.y; }

        constexpr static Vec2 scale(const Vec2& v, float magnitude )
        { return v * Vec2(magnitude); }

        bool operator==(const Vec2& other) const
        { return this->x == other.x && this->y == other.y; }

        static Vec2 round(const Vec2& v )
        { return glm::round( (glm::vec2) v ); }

        static Vec2 lerp( const Vec2& _source, const Vec2& _target, float _factor)
        { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

        static Vec2 transform(const Vec2& v, const glm::mat3& m)
        {
            auto result = m * glm::vec3(v.x, v.y, 1.f);
            return {result.x, result.y};
        }

        static float distance( const Vec2& a, const Vec2&  b )
        { return glm::distance((glm::vec2)a, (glm::vec2)b); }

        static float dot( const Vec2& a, const Vec2&  b )
        { return glm::dot((glm::vec2)a, (glm::vec2)b); }

        static Vec2 normalize(const Vec2& v)
        { return glm::normalize( (glm::vec2)v ); }
    };

    static_assert(std::is_constructible_v<Vec2>);
    static_assert(std::is_copy_constructible_v<Vec2>);
    static_assert(std::is_copy_assignable_v<Vec2>);
    static_assert(std::is_default_constructible_v<Vec2>);
    static_assert(std::is_trivially_copyable_v<Vec2>);
}