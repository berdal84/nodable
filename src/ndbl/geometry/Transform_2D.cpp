#include "Transform_2D.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_transform_2d.hpp"
#undef GLM_ENABLE_EXPERIMENTAL

#include "../Asserts.h"

namespace ndbl
{

void transform2d_set_position(Transform_2D& transform, const Vec2& position)
{
    NDBL_ASSERT(!std::isnan(position.x));
    NDBL_ASSERT(!std::isnan(position.y));

    transform.position            = position;
    transform._matrices_are_dirty = true;
}

Vec2 transform2d_get_position(const Transform_2D& transform)
{
    return transform.position;
}

void transform2d_set_scale(Transform_2D& transform, const Vec2& scale)
{
    NDBL_ASSERT(!std::isnan(scale.x));
    NDBL_ASSERT(!std::isnan(scale.x));
    NDBL_ASSERT(scale.x > 0);
    NDBL_ASSERT(scale.y > 0);

    transform.scale               = scale;
    transform._matrices_are_dirty = true;
}

Vec2 transform2d_get_scale(const Transform_2D& transform)
{
    return transform.scale;
}

const glm::mat3& transform2d_get_matrix(const Transform_2D& transform)
{
    _transform2d_update_matrices(transform);
    return transform._matrix;
}

const glm::mat3& transform2d_get_matrix_inv(const Transform_2D& transform)
{
    _transform2d_update_matrices(transform);
    return transform._matrix_inv;
}

void _transform2d_update_matrices(const Transform_2D& transform)
{
    if ( !transform._matrices_are_dirty )
    {
        return;
    }

    transform._matrices_are_dirty = false;

    transform._matrix = glm::translate(glm::identity<glm::mat3>(), glm::vec2{transform.position} )
                      * glm::rotate(glm::identity<glm::mat3>(), glm::radians(transform.rotation_z))
                      * glm::scale(glm::identity<glm::mat3>(), glm::vec2{transform.scale});

    transform._matrix_inv   = glm::inverse(transform._matrix);
}

}