#pragma once
#include <glm/common.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include "core/types.h"

namespace fw
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

        void round()
        { *this = Vec2::round(*this); }

        static float sqrlen(Vec2 v)
        { return v.x*v.x+v.y*v.y; }

        static Vec2 scale( Vec2 v, float magnitude )
        { return v * Vec2(magnitude); }

        static Vec2 round( Vec2 _vec )
        { return glm::round( (glm::vec2) _vec ); }

        inline static Vec2 lerp( Vec2 _source, Vec2 _target, float _factor)
        { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }

        static Vec2 transform(Vec2 v, glm::mat3 m)
        {
            auto result = m * glm::vec3(v.x, v.y, 1.f);
            return {result.x, result.y};
        }
    };
}