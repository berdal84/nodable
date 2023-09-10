#include "PropertyView.h"
#include "NodeView.h"
#include "SlotView.h"
#include "nodable/core/Node.h"

using namespace ndbl;
using namespace fw;

PropertyView::PropertyView(u8_t _property_id, PoolID<NodeView> _node_view_id)
: property(_property_id)
, show_input(false)
, touched(false)
, node_view(_node_view_id)
{
    FW_ASSERT( _node_view_id.get() != nullptr );
}

void PropertyView::reset()
{
    touched    = false;
    show_input = false;
}
