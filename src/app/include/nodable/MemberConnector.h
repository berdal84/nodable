#pragma once

#include <nodable/Connector.h>
#include <nodable/Nodable.h>
#include <nodable/R.h>
#include <memory> // std::shared_ptr

namespace Nodable {

    // forward declarations
    class MemberView;
    class Member;
    class AppContext;


    /**
     * @brief A MemberConnector represents a physical input or output on a MemberView.
     */
    class MemberConnector: public Connector<MemberConnector>
    {
    public:

        enum class Side
        {
            Top,
            Bottom,
            Left,
            Right
        };

        MemberConnector(const AppContext* _ctx, MemberView* _member, Way _way, Side _pos)
            : m_context(_ctx)
            , m_memberView(_member)
            , m_way(_way)
            , m_display_side(_pos)
        {
            NODABLE_ASSERT(_member)
        };

        ~MemberConnector() = default;
        MemberConnector (const MemberConnector&) = delete;
        MemberConnector& operator= (const MemberConnector&) = delete;

        Member*            get_member()const;
        R::Type_ptr        get_member_type()const;
        vec2               get_pos()const override;
        bool               share_parent_with(const MemberConnector *other)const override;
        bool               has_node_connected() const;
        static bool        draw(const MemberConnector *_connector, float _radius, const ImColor &_color, const ImColor &_borderColor, const ImColor &_hoverColor);
        static void        drop_behavior(bool& require_new_node, bool& has_made_connection);
        static bool        connect(const MemberConnector *_left, const MemberConnector *_right);

        MemberView* m_memberView;
        Side        m_display_side;
        Way         m_way;
        const AppContext* m_context;
        static const MemberConnector* s_hovered;
        static const MemberConnector* s_dragged;
        static const MemberConnector* s_focused;


    };
}