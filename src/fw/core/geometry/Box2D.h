#pragma once
#include "XForm2D.h"

namespace fw
{
    class Box2D
    {
    public:
        Box2D()
        {};

        Box2D(XForm2D _xform, Vec2 _size)
        : m_xform(_xform)
        , m_size(_size)
        {}

        Box2D(const Rect& r)
        : m_xform()
        , m_size()
        {
            m_xform.pos(r.center());
            m_size = r.size();
        }

        XForm2D& xform()
        { return m_xform; }

        const XForm2D& xform() const
        { return m_xform; }

        void size(Vec2 s)
        { m_size = s; }

        Vec2 size() const
        { return m_size; }

        void pos(Vec2 v)
        { m_xform.pos(v); }

        Vec2 pos() const
        { return m_xform.pos(); }

        Rect rect() const // pivot as center
        {
            Vec2 half_size = m_size / 2.f;
            Rect result{-half_size, half_size};
            result.translate(m_xform.pos());
            return result;
        }

        glm::mat3 world_matrix() const
        { return m_xform.world_matrix(); }

        glm::mat3 model_matrix() const
        { return m_xform.model_matrix(); }

    private:
        XForm2D m_xform;
        Vec2    m_size;
    };
}