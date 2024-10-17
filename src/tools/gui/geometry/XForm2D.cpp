#include "XForm2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

tools::XForm2D::~XForm2D()
{
    if ( _parent != nullptr )
        _parent->remove_child( this );

    remove_all_children();
}

tools::Vec2 tools::XForm2D::translate(const tools::Vec2 &p, const tools::XForm2D &xform)
{
    ASSERT( false ) // Not implemented yet
    //glm::vec2 result = xform.get_world_matrix() * glm::vec3(p.x, p.y, 1.f);
    //return result;
}

void tools::XForm2D::set_pos(const Vec2& desired_pos, Space desired_space )
{
    set_world_transform_dirty();
    switch ( desired_space )
    {
        case PARENT_SPACE:
            return _transform.set_position( desired_pos ); // PARENT_SPACE
        case SCREEN_SPACE:
        {
            if (_parent == nullptr)
                return _transform.set_position( desired_pos );
            Vec2 new_pos = Vec2::transform( desired_pos, _parent->get_world_matrix_inv() );
            return _transform.set_position(new_pos); // PARENT_SPACE
        }
        default:
            ASSERT( false ) // Not implemented yet
    }
}

void tools::XForm2D::set_world_transform_dirty()
{
    _world_matrix_dirty = true;
    for (XForm2D* child : _children)
    {
        child->set_world_transform_dirty();
    }
}

void tools::XForm2D::update_world_matrix()
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

tools::Vec2 tools::XForm2D::get_pos(Space space) const
{
    switch ( space )
    {
        case LOCAL_SPACE:
            return {0.f, 0.f};

        case PARENT_SPACE:
            return _transform.get_position();

        case SCREEN_SPACE:
            if ( _parent != nullptr )
                return Vec2::transform(_transform.get_position(), _parent->get_world_matrix() );
            return _transform.get_position();

        default:
            ASSERT(false) // Not implemented yet
    }
}

const glm::mat3 &tools::XForm2D::get_world_matrix() const
{
    const_cast<XForm2D*>(this)->update_world_matrix();
    return _world_matrix;
}


const glm::mat3 &tools::XForm2D::get_world_matrix_inv() const
{
    const_cast<XForm2D*>(this)->update_world_matrix();
    return _world_matrix_inv;
}

tools::XForm2D *tools::XForm2D::get_parent()
{
    return _parent;
}

void tools::XForm2D::add_child(tools::XForm2D* new_child)
{
    ASSERT( new_child != nullptr )
    this->_children.push_back(new_child);
    new_child->_parent = this; // by not changing child's matrix, we preserve parent space coords, xform might move on global space
    new_child->set_world_transform_dirty();
}

void tools::XForm2D::remove_child(tools::XForm2D* existing_child)
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

void tools::XForm2D::remove_all_children()
{
    for( auto each : _children )
    {
        each->_parent = nullptr; // by not changing child's matrix, we preserve parent space coords, xform might move on global space
        each->set_world_transform_dirty();
    }
    _children.clear();
}

void tools::XForm2D::translate(const tools::Vec2& delta, Space space)
{
    ASSERT( space == PARENT_SPACE)
    set_pos( _transform.get_position() + delta, PARENT_SPACE );
}
