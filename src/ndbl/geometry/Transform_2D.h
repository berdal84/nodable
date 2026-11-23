#pragma once
#include "glm/vec2.hpp"
#include "glm/mat3x3.hpp"

#include "Vec2.h"

namespace ndbl
{
    // 2D position/rotation/scale structure associated with some matrix3x3 (cached)
    struct Transform_2D
    {
        Vec2                position   = {0.f, 0.f};
        Vec2                scale      = {1.f, 1.f};
        constexpr static float rotation_z = 0.0f; // I don't need that currently, so it static.

        // Note: the following members are for cache only and should not be used directly

        mutable glm::mat3   _matrix;
        mutable glm::mat3   _matrix_inv;
        mutable bool        _matrices_are_dirty = true;
    };

    void             transform2d_set_position(Transform_2D&, const Vec2&);
    Vec2             transform2d_get_position(const Transform_2D&); //........ alias for transform.position, for API symmetry
    void             transform2d_set_scale(Transform_2D&, const Vec2&);
    Vec2             transform2d_get_scale(const Transform_2D&); //........... alias for transform.scale, for API symmetry
    const glm::mat3& transform2d_get_matrix(const Transform_2D&); //.......... update the matrix if needed and returns it
    const glm::mat3& transform2d_get_matrix_inv(const Transform_2D&); //...... update the matrix if needed and returns it
    
    void             _transform2d_update_matrices(const Transform_2D&);
}