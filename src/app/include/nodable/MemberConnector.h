#pragma once
#include <nodable/AbstractConnector.h>

namespace Nodable {

    // forward declarations
    class MemberView;
    class Member;

    /**
     * @brief A MemberConnector represents a physical input or output on a MemberView.
     */
    class MemberConnector: public AbstractConnector<MemberConnector>
    {
    public:

        MemberConnector(MemberView* _member, Way _way): m_memberView(_member), m_way(_way) {};
        ~MemberConnector() = default;
        Member*            getMember()const;
        ImVec2             getPos()const override;
        bool               hasSameParentWith(const MemberConnector* other)const override;
        bool               hasConnectedNode() const;
        bool               connect(const MemberConnector *other)const override;

        static void        Draw(const MemberConnector*, float _radius, const ImColor &_color, const ImColor &_borderColor, const ImColor &_hoverColor);
        static void        DropBehavior(bool &needsANewNode);

        MemberView* m_memberView;
        Way         m_way;

        static const MemberConnector* s_hovered;
        static const MemberConnector* s_dragged;
        static const MemberConnector* s_focused;

        static void Draw(const MemberConnector *connector);

        static bool Connect(const MemberConnector *_left, const MemberConnector *_right);
    };
}