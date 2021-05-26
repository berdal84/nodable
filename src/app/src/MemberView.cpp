#include <nodable/NodeView.h>
#include <nodable/Member.h>
#include <nodable/MemberConnector.h>

using namespace Nodable;

MemberView::MemberView(Member* _member, NodeView* _nodeView)
        : m_member(_member)
        , m_showInput(false)
        , m_touched(false)
        , m_in(nullptr)
        , m_out(nullptr)
        , m_nodeView(_nodeView)
{
    NODABLE_ASSERT(_member != nullptr); // Member must be defined
    if ( m_member->allowsConnection(Way_In) )   m_in  = new MemberConnector(this, Way_In);
    if ( m_member->allowsConnection(Way_Out) )  m_out = new MemberConnector(this, Way_Out);
}

MemberView::~MemberView()
{
    delete m_in;
    delete m_out;
}
