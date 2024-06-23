#pragma once
#include "XForm2D.h"

namespace tools
{
    // Axis

    constexpr static Vec2 X_AXIS         = {1.f, 0.f};
    constexpr static Vec2 Y_AXIS         = {0.f, 1.f};
    constexpr static Vec2 XY_AXIS        = X_AXIS + Y_AXIS;

    // Pivots

    constexpr static Vec2 CENTER         = {0.f, 0.f};
    constexpr static Vec2 BOTTOM         = Y_AXIS;
    constexpr static Vec2 TOP            = -Y_AXIS;
    constexpr static Vec2 RIGHT          = X_AXIS;
    constexpr static Vec2 LEFT           = -X_AXIS;
    constexpr static Vec2 TOP_LEFT       = LEFT + TOP;
    constexpr static Vec2 TOP_RIGHT      = RIGHT + TOP;
    constexpr static Vec2 BOTTOM_LEFT    = LEFT + BOTTOM;
    constexpr static Vec2 BOTTOM_RIGHT   = RIGHT + BOTTOM;

    class Box
    {
    public:
        Box()
        {};

        Box(XForm2D _xform, const Vec2& _size)
        : m_xform(_xform)
        , m_half_size()
        {
            set_size(_size);
        }

        Box(const Rect& r)
        : m_xform()
        , m_half_size()
        {
            m_xform.pos(r.center());
            set_size(r.size());
        }

        XForm2D& get_xform()
        { return m_xform; }

        const XForm2D& get_xform() const
        { return m_xform; }

        void set_size(const Vec2& s)
        { m_half_size = s * 0.5f; }

        Vec2 get_size() const
        { return m_half_size * 2.0f; }

        void set_pos(const Vec2& v)
        { m_xform.pos(v); }

        Vec2 get_pos() const
        { return m_xform.pos(); }

        Rect get_rect() const
        {
            return {
                get_pivot(TOP_LEFT),
                get_pivot(BOTTOM_RIGHT)
            };
        }

        const glm::mat3& world_matrix() const
        { return m_xform.world_matrix(); }

        const glm::mat3& model_matrix() const
        { return m_xform.model_matrix(); }


        void translate(const Vec2& delta)
        { set_pos( delta + get_pos() );  }

        static Box  transform(const Box& box, const glm::mat3& mat);
        Vec2        get_pivot(const Vec2& pivot) const;

        // Return a follower box aligned on a given leader
        static Box align(
                const Box&  leader,
                const Vec2& leader_pivot,
                const Box&  follower,
                const Vec2& follower_pivot,
                const Vec2& axis = XY_AXIS
                );

    private:
        XForm2D m_xform;
        Vec2    m_half_size;
    };
}