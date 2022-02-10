#include <nodable/NodeView.h>
#include <nodable/Member.h>
#include <nodable/MemberConnector.h>

using namespace Nodable;
using Side = Nodable::MemberConnector::Side;

MemberView::MemberView(const AppContext* _ctx, Member* _member, NodeView* _nodeView)
        : m_member(_member)
        , m_showInput(false)
        , m_touched(false)
        , m_in(nullptr)
        , m_out(nullptr)
        , m_nodeView(_nodeView)
{
    NODABLE_ASSERT(_member ); // Member must be defined
    NODABLE_ASSERT(_nodeView ); // Member must be defined
    if (m_member->allows_connection(Way_In) ) m_in  = new MemberConnector(_ctx, this, Way_In, Side::Top);
    if (m_member->allows_connection(Way_Out) ) m_out = new MemberConnector(_ctx,this, Way_Out, Side::Bottom);
}

MemberView::~MemberView()
{
    delete m_in;
    delete m_out;
}
