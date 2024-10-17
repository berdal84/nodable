#pragma once
#include "glm/vec2.hpp"
#include "glm/glm/mat3x3.hpp"

#include "Vec2.h"

namespace tools
{
    // To edit easily position/rotation/scale and get the matrix3x3 representation
    struct Transform2D
    {
        void             set_position(const Vec2& p);
        void             set_rotation_z(float degrees);
        void             set_scale(const Vec2& p);
        Vec2             get_position() const { return _position; }
        Vec2             get_rotation_z() const { return _rotation_z; }
        Vec2             get_scale() const { return _scale; };
        const glm::mat3& get_matrix() const; // "const": in reality, m_matrix might be updated, but we consider it as a simple cache.
        const glm::mat3& get_matrix_inv() const; // same..

        void             _update_matrix();
        Vec2             _position   = {0.f, 0.f};
        float            _rotation_z = 0.f;
        Vec2             _scale      = {1.f, 1.f};
        glm::mat3        _matrix;
        glm::mat3        _matrix_inv;
        bool             _matrix_dirty = true;
    };
}