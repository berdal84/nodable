#include "Spatial_Node.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

#include "ndbl/Asserts.h"

namespace ndbl 
{

Spatial_Node::~Spatial_Node()
{
    assert( parent == nullptr ); // should have removed this node from its parent
    assert( this->children.size == 0 ); // should have removed all children first
}

void spatialnode_release(Spatial_Node& node)
{
    array_release(node.children);
}

const glm::mat3& spatialnode_world_matrix(const Spatial_Node& node)
{
    spatialnode_update_world_matrix(const_cast<Spatial_Node&>(node));
    return node.cached_world_matrix;
}

const glm::mat3& spatialnode_world_matrix_inv(const Spatial_Node& node)
{
    spatialnode_update_world_matrix(const_cast<Spatial_Node&>(node));
    return node.cached_world_matrix_inv;
}

void spatialnode_clear(Spatial_Node& spatial_node)
{
    while ( spatial_node.children.size != 0 )
    {
        spatialnode_remove_child(spatial_node, *spatial_node.children[0] );
    }
}

void spatialnode_set_position(Spatial_Node& spatial_node, const Vec2& _pos)
{
    transform2d_set_position(spatial_node.transform, _pos ); // PARENT_SPACE
    spatialnode_set_world_transform_dirty(spatial_node);
}

void spatialnode_set_position(Spatial_Node& spatial_node, const Vec2& desired_position, Space desired_space )
{
    switch ( desired_space )
    {
        case PARENT_SPACE:
            spatialnode_set_position(spatial_node, desired_position); // is default
            break;

        case WORLD_SPACE:
        {
            if ( spatial_node.parent == nullptr)
               return spatialnode_set_position(spatial_node, desired_position, PARENT_SPACE); // the world is also the parent space in that case

            glm::vec2 new_pos = spatialnode_world_matrix_inv(*spatial_node.parent) * glm::vec3(desired_position.x, desired_position.y, 1.f);
            spatialnode_set_position(spatial_node, new_pos);
            break;
        }
        default:
            NDBL_UNREACHABLE("Unexpected desired_space");
    }
}

void spatialnode_set_world_transform_dirty(Spatial_Node& spatial_node)
{
    spatial_node.cached_world_matrix_are_dirty = true;
    for (Spatial_Node* child : spatial_node.children)
    {
        spatialnode_set_world_transform_dirty(*child);
    }
}

void spatialnode_update_world_matrix(Spatial_Node& spatial_node)
{
    if ( !spatial_node.cached_world_matrix_are_dirty )
        return;

    if ( spatial_node.parent )
        spatial_node.cached_world_matrix = transform2d_get_matrix(spatial_node.transform) * spatialnode_world_matrix(*spatial_node.parent);
    else
        spatial_node.cached_world_matrix = transform2d_get_matrix(spatial_node.transform);

    spatial_node.cached_world_matrix_inv   = glm::inverse( spatial_node.cached_world_matrix );
    spatial_node.cached_world_matrix_are_dirty = false;
}

Vec2 spatialnode_position(const Spatial_Node& spatial_node)
{
    return transform2d_get_position(spatial_node.transform);
}

Vec2 spatialnode_position(const Spatial_Node& spatial_node, Space space) 
{
    switch ( space )
    {
        default:
            NDBL_UNREACHABLE("This space is not handled");

        case LOCAL_SPACE:
            return {0.f, 0.f};

        case PARENT_SPACE:
            return spatialnode_position(spatial_node); // is default

        case WORLD_SPACE:
            if ( spatial_node.parent == nullptr )
                return spatialnode_position(spatial_node);

            Vec2 position = spatialnode_position(spatial_node);
            glm::vec2 result = spatialnode_world_matrix(*spatial_node.parent)
                             * glm::vec3{ position.x, position.y, 1.0f};
            return result;
    }
}

void spatialnode_add_child(Spatial_Node& parent, Spatial_Node& new_child, Spatial_Node_Flags flags)
{
    NDBL_ASSERT( new_child.parent == nullptr, "Child already has a parent, remove it first from parent" );
    NDBL_ASSERT( &new_child != &parent      , "Adding itself as child" );

    array_append(parent.children, &new_child);

    if ( flags & Spatial_Node_Flag_PRESERVE_WORLD_POSITION )
    {
        const Vec2 world_position = spatialnode_position(new_child, WORLD_SPACE);
        new_child.parent = &parent;
        spatialnode_set_position(new_child, world_position, WORLD_SPACE);
    }
    else
    {
        new_child.parent = &parent;
    }
}

void spatialnode_remove_child(Spatial_Node& spatial_node, Spatial_Node& child, Spatial_Node_Flags flags)
{
    auto found = array_find(spatial_node.children, &child);
    NDBL_ASSERT(found.ok, "Unable to find this child in spatial_node.children");
    array_remove_unordered(spatial_node.children, found.at_pos);

    if ( flags & Spatial_Node_Flag_PRESERVE_WORLD_POSITION )
    {
        const Vec2 pos = spatialnode_position(child, WORLD_SPACE);
        child.parent = nullptr;
        spatialnode_set_position(child, pos, WORLD_SPACE);
    }
    else
    {
        child.parent = nullptr;
    }
}

void spatialnode_translate(Spatial_Node& spatial_node, const Vec2& delta)
{
    // Since Transform2D cannot be rotated yet, we can apply the translation in parent space
    transform2d_set_position(spatial_node.transform, spatial_node.transform.position + delta );
    spatialnode_set_world_transform_dirty(spatial_node);
}

} // namespace ndbl