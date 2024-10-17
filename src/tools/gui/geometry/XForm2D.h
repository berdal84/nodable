#pragma once
#include "glm/glm/mat3x3.hpp"
#include "Rect.h"
#include "Vec2.h"
#include "tools/core/assertions.h"
#include "Transform2D.h"
#include "Space.h"

namespace tools
{
    // Axis

    constexpr static Vec2 X_AXIS         = {1.f, 0.f};
    constexpr static Vec2 Y_AXIS         = {0.f, 1.f};
    constexpr static Vec2 XY_AXIS        = X_AXIS + Y_AXIS;

    // Pivots

    constexpr static Vec2 CENTER         = {0.f, 0.f};
    constexpr static Vec2 BOTTOM         = Y_AXIS;
    constexpr static Vec2 TOP            = -Y_AXIS;
    constexpr static Vec2 RIGHT          = X_AXIS;
    constexpr static Vec2 LEFT           = -X_AXIS;
    constexpr static Vec2 TOP_LEFT       = LEFT + TOP;
    constexpr static Vec2 TOP_RIGHT      = RIGHT + TOP;
    constexpr static Vec2 BOTTOM_LEFT    = LEFT + BOTTOM;
    constexpr static Vec2 BOTTOM_RIGHT   = RIGHT + BOTTOM;

    struct XForm2D
    {
        XForm2D(){};
        ~XForm2D();
        void                  set_pos(const Vec2 &_pos, Space = PARENT_SPACE);
        void                  translate(const Vec2& delta, Space = PARENT_SPACE );
        Vec2                  get_pos(Space = PARENT_SPACE) const;
        const glm::mat3&      get_world_matrix() const;
        const glm::mat3&      get_world_matrix_inv() const;
        void                  set_world_transform_dirty();
        void                  add_child(XForm2D*);
        void                  remove_child(XForm2D* existing_child);
        void                  remove_all_children();
        XForm2D*              get_parent();
        void                  update_world_matrix();
        static Vec2           translate( const Vec2& p, const XForm2D& xform );

        XForm2D*              _parent = nullptr;
        std::vector<XForm2D*> _children;
        Transform2D           _transform; // local transform, relative to the parent
        glm::mat3             _world_matrix = {1.f}; // update only on-demand when m_world_matrix_dirty is set.
        glm::mat3             _world_matrix_inv = {1.f}; // update only on-demand when m_world_matrix_dirty is set.
        bool                  _world_matrix_dirty = true;
    };
}