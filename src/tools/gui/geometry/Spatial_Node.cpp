#include "Spatial_Node.h"
#include "core/Asserts.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

tools::Spatial_Node::~Spatial_Node()
{
    spatialnode_clear(this);
    assert( parent == nullptr ); // remove this instance from its parent before to delete it.
    assert( this->children.empty() ); // _parent is responsible for removing its children before they get delete.
}

const glm::mat3& tools::Spatial_Node::world_matrix() const
{
    spatialnode_update_world_matrix(const_cast<Spatial_Node*>(this)); return cached_world_matrix;
}

const glm::mat3& tools::Spatial_Node::world_matrix_inv() const
{
    spatialnode_update_world_matrix(const_cast<Spatial_Node*>(this)); return cached_world_matrix_inv;
}

void tools::spatialnode_clear(Spatial_Node* spatial_node)
{
    while ( !spatial_node->children.empty() )
    {
        spatialnode_remove_child(spatial_node, *spatial_node->children.begin() ); // TODO: not great to remove one by one from the begining...
    }
}

void tools::spatialnode_set_position(Spatial_Node* spatial_node, const Vec2& _pos)
{
    spatial_node->transform.set_position(_pos ); // PARENT_SPACE
    spatialnode_set_world_transform_dirty(spatial_node);
}

void tools::spatialnode_set_position(Spatial_Node* spatial_node, const Vec2& desired_position, Space desired_space )
{
    switch ( desired_space )
    {
        case PARENT_SPACE:
            spatialnode_set_position(spatial_node, desired_position); // is default
            break;

        case WORLD_SPACE:
        {
            if ( spatial_node->parent == nullptr)
               return spatialnode_set_position(spatial_node, desired_position, PARENT_SPACE); // the world is also the parent space in that case

            glm::vec2 new_pos = spatial_node->parent->world_matrix_inv() * glm::vec3(desired_position.x, desired_position.y, 1.f);
            spatialnode_set_position(spatial_node, new_pos);
            break;
        }
        default:
            ASSERT( false ); // Not implemented yet
    }
}

void tools::spatialnode_set_world_transform_dirty(Spatial_Node* spatial_node)
{
    spatial_node->cached_world_matrix_are_dirty = true;
    for (Spatial_Node* child : spatial_node->children)
    {
        spatialnode_set_world_transform_dirty(child);
    }
}

void tools::spatialnode_update_world_matrix(Spatial_Node* spatial_node)
{
    if ( !spatial_node->cached_world_matrix_are_dirty )
        return;

    if ( spatial_node->parent )
        spatial_node->cached_world_matrix = spatial_node->transform.matrix() * spatial_node->parent->world_matrix();
    else
        spatial_node->cached_world_matrix = spatial_node->transform.matrix();

    spatial_node->cached_world_matrix_inv   = glm::inverse( spatial_node->cached_world_matrix );
    spatial_node->cached_world_matrix_are_dirty = false;
}

tools::Vec2 tools::spatialnode_position(const Spatial_Node* spatial_node)
{
    return spatial_node->transform.position();
}

tools::Vec2 tools::spatialnode_position(const Spatial_Node* spatial_node, Space space) 
{
    switch ( space )
    {
        default:
            VERIFY( false, "This space is not handled");

        case LOCAL_SPACE:
            return {0.f, 0.f};

        case PARENT_SPACE:
            return spatialnode_position(spatial_node); // is default

        case WORLD_SPACE:
            if ( spatial_node->parent == nullptr )
                return spatialnode_position(spatial_node);

            Vec2 position = spatialnode_position(spatial_node);
            glm::vec2 result = spatial_node->parent->world_matrix()
                             * glm::vec3{ position.x, position.y, 1.0f};
            return result;
    }
}

bool tools::spatialnode_add_child(Spatial_Node* parent, Spatial_Node* new_child, Spatial_Node_Flags flags)
{
    ASSERT( new_child != nullptr );
    VERIFY( new_child->parent == nullptr, "Child already has a parent, remove it first from parent" );
    VERIFY( new_child != parent, "Adding itself as primary_child" );

    auto [it, inserted] = parent->children.insert(new_child);

    if ( !inserted )
        return false;

    if ( flags & Spatial_Node_Flag_PRESERVE_WORLD_POSITION )
    {
        const Vec2 world_position = spatialnode_position(new_child, WORLD_SPACE);
        new_child->parent = parent;
        spatialnode_set_position(new_child, world_position, WORLD_SPACE);
    }
    else
    {
        new_child->parent = parent;
    }
    return true;
}

bool tools::spatialnode_remove_child(Spatial_Node* parent, Spatial_Node* child, Spatial_Node_Flags flags)
{
    ASSERT(child);
    if (child->parent == nullptr )
        return false;

    if ( !parent->children.erase(child) )
        return false;

    if ( flags & Spatial_Node_Flag_PRESERVE_WORLD_POSITION )
    {
        const Vec2 pos = spatialnode_position(child, WORLD_SPACE);
        child->parent = nullptr;
        spatialnode_set_position(child, pos, WORLD_SPACE);
    }
    else
    {
        child->parent = nullptr;
    }

    return true;
}

void tools::spatialnode_translate(Spatial_Node* spatial_node, const tools::Vec2& delta)
{
    // Since Transform2D cannot be rotated yet, we can apply the translation in parent space
    spatial_node->transform.set_position(spatial_node->transform.position() + delta );
    spatialnode_set_world_transform_dirty(spatial_node);
}
