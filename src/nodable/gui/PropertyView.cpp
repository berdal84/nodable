#include "PropertyView.h"
#include "NodeView.h"
#include "SlotView.h"
#include "core/language/Nodlang.h"
#include "nodable/core/Node.h"

using namespace ndbl;
using namespace fw;

PropertyView::PropertyView( PoolID<NodeView> _node_view, ID<Property> _id )
: property(_id)
, node_view(_node_view)
, show_input(false)
, touched(false)
{
    bool is_this = get_property()->is_this();
    // Create a SlotView per slot
    for(Slot* slot : get_node()->get_all_slots(_id))
    {
        Side side;
        // This slots (as value) are displayed on the left
        if( is_this && slot->flags & SlotFlag_TYPE_VALUE )
        {
            side = Side::Left;
        }
        // any slot accepting dependencies are displayed on the top
        else if( slot->flags & SlotFlag_ACCEPTS_DEPENDENCIES )
        {
            side = Side::Top;
        }
        else
        {
            side = Side::Bottom;
        }
        slot_views.emplace_back(*slot, side);
    }
}

void PropertyView::reset()
{
    touched    = false;
    show_input = false;
}

Property* PropertyView::get_property() const
{
    return get_node()->get_prop_at(property);
}

Node* PropertyView::get_node() const
{
    return node_view->get_owner().get();
}

std::string PropertyView::serialize_source() const
{
    std::string source_code;
    Property* p = get_property();
    if( p->get_type()->is<PoolID<Node>>() || has_slot( SlotFlag_OUTPUT ))
    {
        Nodlang::get_instance().serialize_node( source_code, node_view->get_owner() );
    }
    return Nodlang::get_instance().serialize_property(source_code, p);
}

bool PropertyView::has_input_connected() const
{
    FW_EXPECT(false, "TODO: implement")
}

bool PropertyView::has_slot( SlotFlag flag ) const
{
    FW_EXPECT(false, "TODO: implement")
}

VariableNode *PropertyView::get_connected_variable() const
{
     FW_EXPECT(false, "TODO: implement")
}

SlotView* PropertyView::get_slot( SlotFlag _flags ) const
{
    for(auto& slot_view : slot_views)
    {
        if( (slot_view.slot().flags & _flags ) == _flags )
        {
            return const_cast<SlotView*>(&slot_view);
        }
    }
    return nullptr;
}

ImVec2 PropertyView::get_pos( ID8<Slot> identifier ) const
{
    for( auto& slot_view : slot_views )
    {
        if( slot_view.slot().id == identifier )
        {
            return slot_view.get_pos();
        }
    }
    LOG_WARNING("PropertyView", "This should not happens");
    return {};
}
