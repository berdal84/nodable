
#pragma once
#include "imgui/imgui.h"
#include "types.h"
#include <cassert>
#include <glm/glm/common.hpp>
#include <glm/glm/vec2.hpp>

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
        constexpr Vec2( const ImVec2& v ): Vec2( v.x, v.y ) {}

        operator ImVec2() const
        { return { x, y }; }

        operator glm::vec2() const
        { return { x, y }; }

#define UNARY_OPERATOR( _OP_ )                \
        Vec2& operator _OP_( const Vec2 & other )\
        {\
            x _OP_ other.x;\
            y _OP_ other.y;\
            return *this;\
        }

        UNARY_OPERATOR( += )
        UNARY_OPERATOR( *= )
        UNARY_OPERATOR( -= )
        UNARY_OPERATOR( /= )

#define BINARY_OPERATOR( _OP_ )                    \
        Vec2 operator _OP_( const Vec2& other ) const\
        { return { x _OP_ other.x, y _OP_ other.y }; }

        BINARY_OPERATOR( +)
        BINARY_OPERATOR( * )
        BINARY_OPERATOR( -)
        BINARY_OPERATOR( / )

        static Vec2 scale( Vec2 v, float magnitude )
        { return v * Vec2(magnitude); }

        static Vec2 round( Vec2 _vec )
        { return glm::round( (glm::vec2) _vec ); }

        inline static Vec2 lerp( Vec2 _source, Vec2 _target, float _factor)
        { return glm::mix((glm::vec2)_source, (glm::vec2)_target, glm::clamp(_factor, 0.0f, 1.0f)); }
    };
}