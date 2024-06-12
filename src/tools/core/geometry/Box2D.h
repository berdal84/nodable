#pragma once
#include "XForm2D.h"

namespace tools
{
    class Box2D
    {
    public:
        Box2D()
        {};

        Box2D(XForm2D _xform, const Vec2& _size)
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

        XForm2D& get_xform()
        { return m_xform; }

        const XForm2D& get_xform() const
        { return m_xform; }

        void set_size(const Vec2& s)
        { m_size = s; }

        Vec2 get_size()
        { return m_size; }

        const Vec2& get_size() const
        { return m_size; }

        void set_pos(const Vec2& v)
        { m_xform.pos(v); }

        Vec2 get_pos() const
        { return m_xform.pos(); }

        Rect get_rect() const // pivot as center
        {
            Vec2 half_size = m_size / 2.f;
            Rect result{-half_size, half_size};
            result.translate(m_xform.pos());
            return result;
        }

        const glm::mat3& world_matrix() const
        { return m_xform.world_matrix(); }

        const glm::mat3& model_matrix() const
        { return m_xform.model_matrix(); }


        void translate(const Vec2& delta)
        { set_pos( delta + get_pos() );  }

        static Box2D transform(const Box2D& box, const glm::mat3& mat)
        {
            Box2D result = box;
            // Translate box position (box shape is relative to it)
            result.set_pos( Vec2::transform( box.get_pos(), mat ) );
            return result;
        }

    private:
        XForm2D m_xform;
        Vec2    m_size;
    };
}