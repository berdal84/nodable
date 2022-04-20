#pragma once

#include <memory> // std::shared_ptr

#include <nodable/app/Connector.h>
#include <nodable/app/types.h>
#include <nodable/core/reflection/type.>
#include <nodable/core/assertions.h>

namespace Nodable {

    // forward declarations
    class MemberView;
    class Member;
    class IAppCtx;

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

        MemberConnector(IAppCtx& _ctx, MemberView* _member, Way _way, Side _pos)
            : m_ctx(_ctx)
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
        type        get_member_type()const;
        vec2               get_pos()const override;
        bool               share_parent_with(const MemberConnector *other)const override;
        bool               has_node_connected() const;
        static void        draw(const MemberConnector *_connector, float _radius, const ImColor &_color, const ImColor &_borderColor, const ImColor &_hoverColor, bool _editable);
        static void        dropped(const MemberConnector *_left, const MemberConnector *_right);

        MemberView* m_memberView;
        Side        m_display_side;
        Way         m_way;
        IAppCtx&    m_ctx;
        static const MemberConnector* s_hovered;
        static const MemberConnector* s_dragged;
        static const MemberConnector* s_focused;


    };
}