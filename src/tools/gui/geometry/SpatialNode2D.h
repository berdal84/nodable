#pragma once
#include "glm/glm/mat3x3.hpp"
#include "Rect.h"
#include "Vec2.h"
#include "tools/core/assertions.h"
#include "TRSTransform2D.h"
#include "Space.h"
#include <unordered_set>

namespace tools
{

    /**
     * Very simple spatial node in 2D.
     * A scene graph can be created by linking parent and child nodes.
     * Currently, we can only set and get the position (not implemented in TRSTransform2D)
     */
    struct SpatialNode2D
    {
        typedef std::unordered_set<SpatialNode2D*> Children;

        SpatialNode2D(){};
        ~SpatialNode2D();
        void                  set_position(const Vec2&);
        void                  set_position(const Vec2&, Space);
        // void                  set_rotate(float angle);
        // void                  rotate(float angle);
        // void                  set_scale(const tools::Vec2& scale);
        // void                  scale(const tools::Vec2& scale);
        Vec2                  position() const;
        Vec2                  position(Space) const;
        void                  translate(const tools::Vec2& delta);
        const glm::mat3&      world_matrix() const     { const_cast<SpatialNode2D*>(this)->update_world_matrix(); return _world_matrix; }
        const glm::mat3&      world_matrix_inv() const { const_cast<SpatialNode2D*>(this)->update_world_matrix(); return _world_matrix_inv; }
        void                  set_world_transform_dirty();
        void                  add_child(SpatialNode2D*);
        void                  remove_child(SpatialNode2D* possible_child);
        bool                  has_parent() const { return _parent != nullptr; }
        SpatialNode2D*        parent() const { return _parent; }
        void                  update_world_matrix();
        const Children&       children() const { return _children; }
    private:
        SpatialNode2D*        _parent = nullptr;
        Children              _children;
        TRSTransform2D        _transform; // local transform, relative to the parent
        glm::mat3             _world_matrix = {1.f}; // update only on-demand when m_world_matrix_dirty is set.
        glm::mat3             _world_matrix_inv = {1.f}; // update only on-demand when m_world_matrix_dirty is set.
        bool                  _world_matrix_dirty = true;
    };
}