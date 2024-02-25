#include "Node.h"
#include "ForLoopNode.h"
#include "core/Scope.h"

#include <utility>
#include <algorithm> // for std::find

#include "Graph.h"
#include "GraphUtil.h"
#include "InvokableComponent.h"

using namespace ndbl;
using fw::Pool;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<Node>("Node");
}

Node::Node(std::string _label)
: name(std::move(_label))
, dirty(true)
, flagged_to_delete(false)
, parent_graph( nullptr )
, m_this_property_id( add_prop<PoolID<Node>>( THIS_PROPERTY ) ) // Add a property acting like a "this" for the owner Node.
{
}

Node::Node( Node&& other ) noexcept
: name(std::move(other.name))
, dirty(other.dirty)
, flagged_to_delete(other.flagged_to_delete)
, props(std::move(other.props))
, m_slots(std::move(other.m_slots))
, m_components(std::move(other.m_components))
, m_id( other.m_id )
, parent_graph( other.parent_graph )
, m_this_property_id( other.m_this_property_id )
, after_token(other.after_token)
{
}

Node& Node::operator=( Node&& other ) noexcept
{
   if( this == &other ) return *this;

   name              = std::move(other.name);
   dirty             = other.dirty;
   flagged_to_delete = other.flagged_to_delete;
   props             = std::move(other.props);
   m_slots           = std::move(other.m_slots);
   m_components      = std::move(other.m_components);
   m_this_property_id = other.m_this_property_id;
   m_id              = other.m_id;
   after_token       = std::move(other.after_token);
   parent_graph      = other.parent_graph;

   return *this;
}

void Node::init()
{
    get_prop_at( m_this_property_id )->set(m_id);

    add_slot( SlotFlag_PARENT, 1);
    add_slot( SlotFlag_NEXT, 1);

    m_components.set_owner( m_id );
}

size_t Node::adjacent_slot_count(SlotFlags _flags )const
{
    return filter_adjacent_slots( _flags ).size();
}

const fw::iinvokable* Node::get_connected_invokable(const char* property_name) const
{
    const Slot&   slot          = *find_slot_by_property_name( property_name, SlotFlag_INPUT );
    const SlotRef adjacent_slot = slot.first_adjacent();

    if ( adjacent_slot )
    {
        if ( auto* invokable = adjacent_slot.node->get_component<InvokableComponent>().get() )
        {
            return invokable->get_function();
        }
    }
    return nullptr;
}

void Node::set_name(const char *_label)
{
    name = _label;
    on_name_change.emit(m_id);
}

std::vector<PoolID<Component>> Node::get_components()
{
    return m_components.get_all();
}

Slot* Node::find_slot_by_property_name(const char* _property_name, SlotFlags _flags)
{
    return const_cast<Slot*>( const_cast<const Node*>(this)->find_slot_by_property_name( _property_name, _flags));
}

const Slot* Node::find_slot_by_property_name(const char* property_name, SlotFlags desired_way) const
{
    const Property* property = get_prop(property_name);
    if( property )
    {
        return find_slot_by_property_id( property->id, desired_way );
    }
    return nullptr;
}

const Property* Node::get_prop(const char *_name) const
{
    return props.find_by_name( _name );
}

Property* Node::get_prop(const char *_name)
{
    return props.find_by_name( _name );
}

Property* Node::get_prop_at(ID<Property> id)
{
    return props.at(id);
}

const Property* Node::get_prop_at(ID<Property> id) const
{
    return props.at(id);
}

Slot* Node::find_slot(SlotFlags _flags)
{
    return const_cast<Slot*>( const_cast<const Node*>(this)->find_slot(_flags));
}

const Slot* Node::find_slot(SlotFlags _flags) const
{
    return find_slot_by_property_id( m_this_property_id, _flags );
}

Slot* Node::find_slot_at(SlotFlags _flags, size_t _position)
{
    return const_cast<Slot*>( const_cast<const Node*>(this)->find_slot_at(_flags, _position));
}

const Slot* Node::find_slot_at(SlotFlags _flags, size_t _position) const
{
    for( const Slot& slot : m_slots )
    {
        if( slot.has_flags(_flags) && slot.position() == _position && slot.property == m_this_property_id )
        {
            return &slot;
        }
    }
    return nullptr;
}

Slot& Node::get_slot_at(ID8<Slot> id)
{
    return m_slots[id.m_value];
}

const Slot& Node::get_slot_at(ID8<Slot> id) const
{
    return m_slots[id.m_value];
}

std::vector<PoolID<Node>> Node::outputs() const
{
    return filter_adjacent(SlotFlag_OUTPUT);
}

std::vector<PoolID<Node>> Node::inputs() const
{
    return filter_adjacent(SlotFlag_INPUT);
}

std::vector<PoolID<Node>> Node::predecessors() const
{
    return filter_adjacent(SlotFlag_PREV);
}

std::vector<PoolID<Node>> Node::rchildren() const
{
    auto v = children();
    std::reverse( v.begin(), v.end() );
    return v;
}

std::vector<PoolID<Node>> Node::children() const
{
    return filter_adjacent(SlotFlag_CHILD);
}

std::vector<PoolID<Node>> Node::successors() const
{
    return filter_adjacent(SlotFlag_NEXT);
}

std::vector<PoolID<Node>> Node::filter_adjacent( SlotFlags _flags ) const
{
    return GraphUtil::get_adjacent_nodes(this, _flags);
}

Slot* Node::find_slot_by_property_type(SlotFlags flags, const fw::type* _type)
{
    for(Slot* slot : filter_slots( flags ) )
    {
        if( fw::type::is_implicitly_convertible( slot->get_property()->get_type(), _type ) )
        {
            return slot;
        }
    }
    return nullptr;
}

Slot & Node::get_nth_slot( u8_t _n, SlotFlags _flags )
{
    u8_t count = 0;
    for( auto& slot : m_slots )
    {
        if ( slot.has_flags(_flags) )
        {
            if( count == _n )
            {
                return slot;
            }
            count++;
        }
    }
    FW_EXPECT(false, "Not found")
}

ID<Property> Node::add_prop(const fw::type *_type, const char *_name, PropertyFlags _flags)
{
    return props.add(_type, _name, _flags);
}

ID8<Slot> Node::add_slot(SlotFlags _flags, u8_t _capacity, ID<Property> _prop_id)
{
    Slot& slot = m_slots.emplace_back( (u8_t)m_slots.size(), m_id, _flags, _prop_id, _capacity );
    return slot.id;
}

ID8<Slot> Node::add_slot(SlotFlags _flags, u8_t _capacity, size_t _position)
{
    Slot& slot = m_slots.emplace_back( (u8_t)m_slots.size(), m_id, _flags, m_this_property_id, _capacity, _position );
    return slot.id;
}

PoolID<Node> Node::find_parent() const
{
    if ( const Slot* parent_slot = find_slot( SlotFlag_PARENT ) )
    {
        return parent_slot->first_adjacent().node;
    }
    return {};
}

std::vector<SlotRef> Node::filter_adjacent_slots( SlotFlags _flags ) const
{
    std::vector<SlotRef> result;
    for(const Slot* slot : filter_slots(_flags))
    {
        for( const SlotRef& each : slot->adjacent() )
        {
            result.emplace_back( each );
        }
    }
    return result;
}

bool Node::has_input_connected( const ID<Property>& id ) const
{
    const Slot* slot = find_slot_by_property_id( id, SlotFlag_INPUT );
    return slot && slot->adjacent_count() > 0;
}

size_t Node::slot_count(SlotFlags flags) const
{
    return filter_slots( flags ).size();
}

Slot* Node::find_slot_by_property_id(ID<Property> property_id, SlotFlags _flags)
{
    return const_cast<Slot*>( const_cast<const Node*>( this )->find_slot_by_property_id( property_id, _flags ) );
}

const Slot* Node::find_slot_by_property_id(ID<Property> property_id, SlotFlags _flags) const
{
    for(auto& slot : m_slots )
    {
        if( slot.has_flags(_flags) && slot.property == property_id )
        {
            return &slot;
        }
    }
    return nullptr;
}

Slot* Node::find_adjacent_at( SlotFlags _flags, u8_t _index ) const
{
    size_t cursor_pos{0};
    for (auto& slot : m_slots)
    {
        // Skip any slot not compatible with given flags
        if( !slot.has_flags( _flags ) )
        {
            continue;
        }

        // if the position is in the range of this slot, we return the item
        size_t local_pos = (size_t)_index - cursor_pos;
        if ( local_pos < slot.adjacent_count() )
        {
            return slot.adjacent_at(local_pos).get();
        }
        // increase counter
        cursor_pos += slot.adjacent_count();
    }
    return nullptr;
}

std::vector<Slot*> Node::filter_slots( SlotFlags _flags ) const
{
    std::vector<Slot*> result;
    for(auto& slot : m_slots)
    {
        if( slot.has_flags(_flags) )
        {
            result.push_back(const_cast<Slot*>( &slot ));
        }
    }
    return result;
}

bool Node::is_instruction() const
{
    if( !predecessors().empty())
    {
        return true;
    }
    return find_slot_by_property_name( THIS_PROPERTY, SlotFlag_OUTPUT | SlotFlag_NOT_FULL) == nullptr;
}

bool Node::should_be_constrain_to_follow_output( PoolID<const Node> _output ) const
{
    const auto& _outputs = outputs();
    return predecessors().empty() && _outputs.size() >= 1 && _outputs.back() == _output->m_id;
}
