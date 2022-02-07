#pragma once
#include <nodable/AbstractConnector.h>
#include <nodable/Nodable.h>

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

        enum class Side
        {
            Top,
            Bottom,
            Left,
            Right
        };

        MemberConnector(MemberView* _member, Way _way, Side _pos)
            : m_memberView(_member)
            , m_way(_way)
            , m_display_side(_pos)
        {
            NODABLE_ASSERT(_member)
        };

        ~MemberConnector() = default;
        MemberConnector (const MemberConnector&) = delete;
        MemberConnector& operator= (const MemberConnector&) = delete;

        Member*            getMember()const;
        ImVec2             getPos()const override;
        bool               hasSameParentWith(const MemberConnector*)const override;
        bool               hasConnectedNode() const;
        bool               connect(const MemberConnector*)const override;

        static void        Draw(const MemberConnector*, float _radius, const ImColor &_color, const ImColor &_borderColor, const ImColor &_hoverColor);
        static void        DropBehavior(bool &needsANewNode);

        MemberView* m_memberView;
        Side        m_display_side;
        Way         m_way;
        static const MemberConnector* s_hovered;
        static const MemberConnector* s_dragged;
        static const MemberConnector* s_focused;

        static void Draw(const MemberConnector *connector);

        static bool Connect(const MemberConnector *_left, const MemberConnector *_right);
    };
}