#pragma once

#include "Transform_2D.h"
#include "Space.h"
#include "Vec2.h"

namespace ndbl
{

    typedef int Spatial_Node_Flags;
    enum Spatial_Node_Flag_
    {
        Spatial_Node_Flag_NONE                    = 0,
        Spatial_Node_Flag_PRESERVE_WORLD_POSITION = 1 << 0,
    };

    /**
     * Very simple spatial node in 2D.
     * A scene graph can be created by linking parent and primary_child nodes.
     * Currently, we can only set and get the position (not implemented in Transform_2D)
     */
    struct Spatial_Node
    {
        Spatial_Node*   parent                          = nullptr;
        Spatial_Node*   first                           = nullptr; // intrusive linked-list
        Spatial_Node*   last                            = nullptr; // intrusive linked-list
        Spatial_Node*   prev                            = nullptr; // intrusive linked-list
        Spatial_Node*   next                            = nullptr; // intrusive linked-list
        Transform_2D    transform                       = {}; //..... relative to the parent, if any.
        glm::mat3       _cached_world_matrix            = {1.f}; //.. use world_matrix() if you want to update the matrix before to get it.
        glm::mat3       _cached_world_matrix_inv        = {1.f}; //.. use world_matrix_inv() if you want to update the matrix before to get it.
        bool            _cached_world_matrix_are_dirty  = true; //... when true, this force _world_matrix and _word_matrix_inv to be updated.
    };

    const glm::mat3&    spatialnode_world_matrix(const Spatial_Node&);
    const glm::mat3&    spatialnode_world_matrix_inv(const Spatial_Node&);
    void                spatialnode_set_position(Spatial_Node&, const Vec2&);
    void                spatialnode_set_position(Spatial_Node&, const Vec2&, Space);
    Vec2                spatialnode_position(const Spatial_Node&);
    Vec2                spatialnode_position(const Spatial_Node&, Space);
    void                spatialnode_translate(Spatial_Node&, const Vec2& delta);
    void                spatialnode_set_world_transform_dirty(Spatial_Node&);
    void                spatialnode_add_child(Spatial_Node& /* parent */, Spatial_Node& /* new_child */, Spatial_Node_Flags = Spatial_Node_Flag_PRESERVE_WORLD_POSITION);
    void                spatialnode_remove_child(Spatial_Node& /* parent */, Spatial_Node& /* existing_child */, Spatial_Node_Flags = Spatial_Node_Flag_PRESERVE_WORLD_POSITION);
    void                spatialnode_update_world_matrix(Spatial_Node&);
    void                spatialnode_clear(Spatial_Node&);

}