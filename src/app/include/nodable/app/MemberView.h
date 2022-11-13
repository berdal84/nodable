#include <nodable/app/MemberConnector.h>
#include <nodable/core/types.h>

namespace ndbl {
    class NodeView;

    /**
     * Simple struct to store a member view state
     */
    class MemberView
    {
        vec2              m_relative_pos;

    public:
        using ConnectorPtr = s_ptr<MemberConnector>;

        Member*      m_member;
        NodeView*    m_nodeView;
        ConnectorPtr m_in;
        ConnectorPtr m_out;
        bool         m_showInput;
        bool         m_touched;

        MemberView(IAppCtx& _ctx, Member* _member, NodeView* _nodeView);
        MemberView (const MemberView&) = delete;
        MemberView& operator= (const MemberView&) = delete;

        void reset();
        vec2 relative_pos() const;
        void relative_pos(vec2 _pos);
    };
}