#include "TRSTransform2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

#include "tools/core/assertions.h"

using namespace tools;

void TRSTransform2D::set_position(const Vec2& p)
{
    ASSERT(!std::isnan(p.x));
    ASSERT(!std::isnan(p.x));
    _position     = p;
    _matrix_dirty = true;
}

//void Transform2D::set_rotation_z(float angle)
//{
//    _rotation_z   = angle;
//    _matrix_dirty = true;
//}
//
//void Transform2D::set_scale(const Vec2& scale)
//{
//    _scale        = scale;
//    _matrix_dirty = true;
//}

const glm::mat3 &TRSTransform2D::get_matrix() const
{
    const_cast<TRSTransform2D*>(this)->_update_matrix();
    return _matrix;
}

const glm::mat3 &TRSTransform2D::get_matrix_inv() const
{
    const_cast<TRSTransform2D*>(this)->_update_matrix();
    return _matrix_inv;
}

void TRSTransform2D::_update_matrix()
{
    if ( !_matrix_dirty )
        return;

    _matrix_dirty = false;

    _matrix       = glm::translate(glm::identity<glm::mat3>(), glm::vec2{_position} )
                  * glm::rotate(glm::identity<glm::mat3>(), glm::radians(_rotation_z))
                  * glm::scale(glm::identity<glm::mat3>(), glm::vec2{_scale});

    _matrix_inv   = glm::inverse(_matrix);
}
