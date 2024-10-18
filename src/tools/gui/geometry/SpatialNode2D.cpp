#include "SpatialNode2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

tools::SpatialNode2D::~SpatialNode2D()
{
    if ( _parent != nullptr )
        _parent->remove_child( this );

    remove_all_children();
}


void tools::SpatialNode2D::set_pos(const Vec2& desired_pos)
{
    _transform.set_position( desired_pos ); // PARENT_SPACE
    set_world_transform_dirty();
}

void tools::SpatialNode2D::set_pos(const Vec2& desired_pos, Space desired_space )
{
    switch ( desired_space )
    {
        case PARENT_SPACE:
            set_pos(desired_pos); // is default
            break;

        case WORLD_SPACE:
        {
            if (_parent == nullptr)
               return set_pos(desired_pos, PARENT_SPACE); // the world is also the parent space in that case

            glm::vec2 new_pos = _parent->get_world_matrix_inv() * glm::vec3(desired_pos.x, desired_pos.y, 1.f);
            set_pos(new_pos);
            break;
        }
        default:
            ASSERT( false ) // Not implemented yet
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
        _world_matrix = _transform.get_matrix() * _parent->get_world_matrix();
    else
        _world_matrix = _transform.get_matrix();

    _world_matrix_inv   = glm::inverse( _world_matrix );
    _world_matrix_dirty = false;
}

tools::Vec2 tools::SpatialNode2D::get_pos() const
{
    return _transform.get_position();
}

tools::Vec2 tools::SpatialNode2D::get_pos(Space space) const
{
    switch ( space )
    {
        default:
            VERIFY( space != LOCAL_SPACE, "Are you sure this is what you want to do? It would return Vec2(0.f,0.f) ")
            VERIFY( false, "This space is not handled")

        case PARENT_SPACE:
            return get_pos(); // is default

        case WORLD_SPACE:
            if ( _parent == nullptr )
                return get_pos();

            glm::vec2 result = _parent->get_world_matrix() * glm::vec3{ get_pos().x, get_pos().y, 1.0f};
            return result;
    }
}

const glm::mat3 &tools::SpatialNode2D::get_world_matrix() const
{
    const_cast<SpatialNode2D*>(this)->update_world_matrix();
    return _world_matrix;
}


const glm::mat3 &tools::SpatialNode2D::get_world_matrix_inv() const
{
    const_cast<SpatialNode2D*>(this)->update_world_matrix();
    return _world_matrix_inv;
}

tools::SpatialNode2D *tools::SpatialNode2D::get_parent()
{
    return _parent;
}

void tools::SpatialNode2D::add_child(tools::SpatialNode2D* new_child)
{
    ASSERT( new_child != nullptr )
    this->_children.push_back(new_child);
    new_child->_parent = this; // by not changing child's matrix, we preserve parent space coords, xform might move on global space
    new_child->set_world_transform_dirty();
}

void tools::SpatialNode2D::remove_child(tools::SpatialNode2D* existing_child)
{
    for(auto it = _children.begin(); it != _children.end(); ++it )
        if ( existing_child == *it )
        {
            _children.erase(it);
            existing_child->_parent = nullptr; // by not changing child's matrix, we preserve parent space coords, xform might move on global space
            existing_child->set_world_transform_dirty();
            return;
        }
    ASSERT(false) // Unable to find child
}

void tools::SpatialNode2D::remove_all_children()
{
    for( auto each : _children )
    {
        each->_parent = nullptr; // by not changing child's matrix, we preserve parent space coords, xform might move on global space
        each->set_world_transform_dirty();
    }
    _children.clear();
}

void tools::SpatialNode2D::translate(const tools::Vec2& delta)
{
    // Since Transform2D cannot be rotated yet, we can apply the translation in parent space
    _transform.set_position( _transform.get_position() + delta );
    set_world_transform_dirty();
}