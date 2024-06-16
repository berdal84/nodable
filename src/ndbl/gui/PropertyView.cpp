#include "PropertyView.h"

#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/Node.h"
#include "NodeView.h"

using namespace ndbl;
using namespace tools;

PropertyView::PropertyView( NodeView* _node_view, Property* _property )
: tools::View(_node_view)
, m_node_view(_node_view)
, m_property(_property)
, show_input(false)
, touched(false)
{
}

void PropertyView::reset()
{
    touched    = false;
    show_input = false;
}

Property* PropertyView::get_property() const
{
    return m_property;
}

Node* PropertyView::get_node() const
{
    return m_node_view->get_owner();
}

bool PropertyView::has_input_connected() const
{
    return get_node()->has_input_connected( m_property );
}

VariableNode* PropertyView::get_connected_variable() const
{
    const Slot* input_slot = get_node()->find_slot_by_property( m_property, SlotFlag_INPUT );
    if( !input_slot )
    {
        return nullptr;
    }

    Slot* adjacent_slot = input_slot->first_adjacent();
    if( adjacent_slot == nullptr )
    {
        return nullptr;
    }

    return cast<VariableNode>( adjacent_slot->get_node() );
}

