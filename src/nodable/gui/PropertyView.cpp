#include "PropertyView.h"
#include "NodeView.h"
#include "SlotView.h"
#include "core/language/Nodlang.h"
#include "nodable/core/Node.h"

using namespace ndbl;
using namespace fw;

PropertyView::PropertyView()
{
}

PropertyView::PropertyView( PoolID<NodeView> _node_view, ID<Property> _id )
: property_id(_id)
, node_view(_node_view)
, show_input(false)
, touched(false)
{
}

PropertyView::PropertyView( const PropertyView& other )
: screen_rect( other.screen_rect )
, show_input( other.show_input )
, touched( other.touched )
, node_view( other.node_view )
, property_id( other.property_id )
{
}

void PropertyView::reset()
{
    touched    = false;
    show_input = false;
}

Property* PropertyView::get_property() const
{
    return get_node()->get_prop_at( property_id );
}

Node* PropertyView::get_node() const
{
    return node_view->get_owner().get();
}

bool PropertyView::has_input_connected() const
{
    return get_node()->has_input_connected( property_id );
}

VariableNode* PropertyView::get_connected_variable() const
{
    const Slot* input_slot = get_node()->find_slot_by_property_id( property_id, SlotFlag_INPUT );
    if( !input_slot )
    {
        return nullptr;
    }

    Slot* adjacent_slot = input_slot->first_adjacent().get();
    if( adjacent_slot == nullptr )
    {
        return nullptr;
    }

    return fw::cast<VariableNode>( adjacent_slot->node.get() );
}