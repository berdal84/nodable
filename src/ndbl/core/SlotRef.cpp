#include "SlotRef.h"
#include "Slot.h"
#include "Node.h"

using namespace ndbl;

const SlotRef SlotRef::null{};

SlotRef::SlotRef()
: node()
, id()
, flags(SlotFlag_NONE)
{
}

SlotRef::SlotRef( const SlotRef& other )
: node( other.node )
, id( other.id )
, flags( other.flags )
{
}

SlotRef::SlotRef(SlotRef&& other) noexcept
: node(other.node)
, id(other.id)
, flags(other.flags)
{
}

SlotRef::SlotRef( const Slot& slot )
: id(slot.id)
, node(slot.node)
, flags(slot.type_and_order() )
{
}

bool SlotRef::operator==( const SlotRef &other ) const
{
    return node == other.node && id == other.id;
}

bool SlotRef::operator!=( const SlotRef &other ) const
{
    return !node || !id || node != other.node || id != other.id;
}

SlotRef& SlotRef::operator=(SlotRef&& other)
{
    id    = std::move( other.id );
    node  = std::move( other.node );
    flags = std::move( other.flags );

    return *this;
}

SlotRef& SlotRef::operator=(const SlotRef& other)
{
    node  = other.node;
    id    = other.id;
    flags = other.flags;

    return *this;
}

Slot* SlotRef::operator->() const
{
    return get();
}

Slot* SlotRef::get() const
{
    Node* _node = node.get();
    if( _node == nullptr )
    {
        return nullptr;
    }
    return &_node->get_slot_at( id );
}
SlotFlags SlotRef::slot_type() const
{
    return flags & SlotFlag_TYPE_MASK;
}
