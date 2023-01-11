#include <nodable/app/NodeView.h>
#include <nodable/core/Member.h>
#include <nodable/app/MemberConnector.h>

using namespace ndbl;
using Side = ndbl::MemberConnector::Side;

MemberView::MemberView(IAppCtx& _ctx, Member* _member, NodeView* _nodeView)
        : m_member(_member)
        , m_showInput(false)
        , m_touched(false)
        , m_in(nullptr)
        , m_out(nullptr)
        , m_nodeView(_nodeView)
{
    NDBL_ASSERT(_member ); // Member must be defined
    NDBL_ASSERT(_nodeView ); // Member must be defined
    if (m_member->allows_connection(Way_In) ) m_in  = new MemberConnector(_ctx, this, Way_In, Side::Top);
    if (m_member->allows_connection(Way_Out) ) m_out = new MemberConnector(_ctx,this, Way_Out, Side::Bottom);
}

MemberView::~MemberView()
{
    delete m_in;
    delete m_out;
}
