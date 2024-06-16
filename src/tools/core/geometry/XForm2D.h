#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#undef GLM_ENABLE_EXPERIMENTAL

#include "Rect.h"
#include "Vec2.h"
#include "tools/core/assertions.h"

namespace tools
{
    class XForm2D
    {
    public:
        XForm2D(){};

        void pos(const Vec2& _pos)
        {
            EXPECT(!std::isnan(_pos.x) && !std::isnan(_pos.y), "Vector can't have NaN coordinates");
            m_pos = _pos;
            m_matrix_are_dirty = true;
        }

        Vec2 pos() const
        { return m_pos; }

        const glm::mat3& world_matrix() const
        { const_cast<XForm2D*>(this)->update(); return m_world_mat; }

        const glm::mat3& model_matrix() const
        { const_cast<XForm2D*>(this)->update(); return m_local_mat; }

        static Vec2 translate( const Vec2& p, const XForm2D& xform )
        {
            glm::vec2 result = xform.world_matrix() * glm::vec3(p.x, p.y, 1.f);
            return result;
        }

    private:
        bool      m_matrix_are_dirty{true};
        glm::vec2 m_pos{0.f, 0.f};
        glm::mat3 m_world_mat{1.f};
        glm::mat3 m_local_mat{1.f};

        void update()
        {
            if ( !m_matrix_are_dirty )
                return;

            glm::mat3 m = glm::translate( glm::mat3{}, m_pos);
            m_local_mat = glm::inverse( m_world_mat );

            m_matrix_are_dirty = false;
        }
    };
}