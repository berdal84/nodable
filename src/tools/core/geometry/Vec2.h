#pragma once

#include <glm/common.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec2.hpp>

#include "tools/core/types.h"

namespace tools
{
    class Vec2
    {
    public:
        float x{};
        float y{};

        constexpr Vec2() = default;
        constexpr Vec2( float x, float y ): x( x ), y( y ) {}
        constexpr Vec2( const Vec2& v ): Vec2( v.x, v.y ) {}
        constexpr Vec2( const glm::vec2& v ): Vec2( v.x, v.y ) {}
        explicit constexpr Vec2( float scalar ): Vec2( scalar, scalar ) {}

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

        inline static Vec2 lerp( Vec2 _source, Vec2 _target, float _factor)
        { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

        inline static Vec2 transform(Vec2 v, glm::mat3 m)
        {
            auto result = m * glm::vec3(v.x, v.y, 1.f);
            return {result.x, result.y};
        }

        inline static float distance( Vec2 a, Vec2 b )
        { return glm::distance((glm::vec2)a, (glm::vec2)b); }

        inline static const float dot( Vec2 a, Vec2 b )
        { return glm::dot((glm::vec2)a, (glm::vec2)b); }
    };
}