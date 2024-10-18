#include "PropertyView.h"

#include "ndbl/core/language/Nodlang.h"
#include "ndbl/core/Node.h"
#include "NodeView.h"

using namespace ndbl;
using namespace tools;

PropertyView::PropertyView(Property* _property )
: m_property(_property)
, show_input(false)
, touched(false)
, m_view_state(10.f, 10.f)
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
    return m_property->owner();
}

bool PropertyView::has_input_connected() const
{
    return get_node()->has_input_connected( m_property );
}

Slot* PropertyView::get_connected_slot() const
{
    const Slot* input_slot = get_node()->find_slot_by_property( m_property, SlotFlag_INPUT );
    if( !input_slot )
        return nullptr;

    return input_slot->first_adjacent();
}

VariableNode* PropertyView::get_connected_variable() const
{
    Slot* adjacent_slot = get_connected_slot();
    if( !adjacent_slot )
        return nullptr;

    return cast<VariableNode>(adjacent_slot->node() );
}
