#include "SpatialNode2D.h"
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

tools::SpatialNode2D::~SpatialNode2D()
{
    if ( _parent != nullptr )
        _parent->remove_child( this );

    while ( !_children.empty() )
        remove_child( *_children.begin() );
}


void tools::SpatialNode2D::set_position(const Vec2& _pos)
{
    _transform.set_position(_pos ); // PARENT_SPACE
    set_world_transform_dirty();
}

void tools::SpatialNode2D::set_position(const Vec2& _pos, Space desired_space )
{
    switch ( desired_space )
    {
        case PARENT_SPACE:
            set_position(_pos); // is default
            break;

        case WORLD_SPACE:
        {
            if (_parent == nullptr)
               return set_position(_pos, PARENT_SPACE); // the world is also the parent space in that case

            glm::vec2 new_pos = _parent->world_matrix_inv() * glm::vec3(_pos.x, _pos.y, 1.f);
            set_position(new_pos);
            break;
        }
        default:
            ASSERT( false ); // Not implemented yet
    }
}

void tools::SpatialNode2D::set_world_transform_dirty()
{
    _world_matrix_dirty = true;
    for (SpatialNode2D* child : _children)
    {
        child->set_world_transform_dirty();
    }
}

void tools::SpatialNode2D::update_world_matrix()
{
    if ( !_world_matrix_dirty )
        return;

    if ( _parent )
        _world_matrix = _transform.matrix() * _parent->world_matrix();
    else
        _world_matrix = _transform.matrix();

    _world_matrix_inv   = glm::inverse( _world_matrix );
    _world_matrix_dirty = false;
}

tools::Vec2 tools::SpatialNode2D::position() const
{
    return _transform.position();
}

tools::Vec2 tools::SpatialNode2D::position(Space space) const
{
    switch ( space )
    {
        default:
            VERIFY( space != LOCAL_SPACE, "Are you sure this is what you want to do? It would return Vec2(0.f,0.f) ");
            VERIFY( false, "This space is not handled");

        case PARENT_SPACE:
            return position(); // is default

        case WORLD_SPACE:
            if ( _parent == nullptr )
                return position();

            glm::vec2 result = _parent->world_matrix() * glm::vec3{position().x, position().y, 1.0f};
            return result;
    }
}

void tools::SpatialNode2D::add_child(tools::SpatialNode2D* new_child)
{
    ASSERT( new_child != nullptr );
    ASSERT( new_child->parent() == nullptr );
    auto [it, success] = this->_children.insert(new_child);
    VERIFY( success, "Unable to insert new_child");
    const Vec2 world_position = new_child->position(WORLD_SPACE);
    new_child->_parent = this;
    new_child->set_position(world_position, WORLD_SPACE);
}

void tools::SpatialNode2D::remove_child(tools::SpatialNode2D* possible_child)
{
    const int success = _children.erase(possible_child);
    VERIFY( success, "Unable to erase possible_child");
    const Vec2 world_position = possible_child->position(WORLD_SPACE);
    possible_child->_parent = nullptr;
    possible_child->set_position(world_position, WORLD_SPACE);
    return;
}

void tools::SpatialNode2D::translate(const tools::Vec2& delta)
{
    // Since Transform2D cannot be rotated yet, we can apply the translation in parent space
    _transform.set_position( _transform.position() + delta );
    set_world_transform_dirty();
}
