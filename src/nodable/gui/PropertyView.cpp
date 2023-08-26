#include "PropertyView.h"
#include "nodable/core/Property.h"
#include "nodable/core/Node.h"
#include "PropertyConnector.h"
#include "NodeView.h"

using namespace ndbl;
using namespace fw::pool;
using Side = ndbl::PropertyConnector::Side;

PropertyView::PropertyView(Property* _property, ID<NodeView> _nodeView)
        : m_property(_property)
        , show_input(false)
        , touched(false)
        , m_input(nullptr)
        , m_output(nullptr)
        , node_view(_nodeView)
{
    FW_ASSERT( _property != nullptr );
    FW_ASSERT( _nodeView.get() != nullptr );
    if (m_property->allows_connection(Way_In)  ) m_input  = new PropertyConnector(this, Way_In, Side::Top);
    if (m_property->allows_connection(Way_Out) ) m_output = new PropertyConnector(this, Way_Out, Side::Bottom);
}

PropertyView::~PropertyView()
{
    delete m_input;
    delete m_output;
}

void PropertyView::reset()
{
    touched    = false;
    show_input = false;
}
