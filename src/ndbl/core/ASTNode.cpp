#include "ASTNode.h"

#include <algorithm> // for std::find

#include "ASTScope.h"
#include "ASTGraph.h"
#include "ASTUtils.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ASTNode>("Node");
}

ASTNode::~ASTNode()
{
    for(auto* each : m_slots)
        delete each;
}

void ASTNode::init(ASTNodeType _type, const std::string& _label)
{
    m_props.init(this);
    m_value = add_prop<null>(DEFAULT_PROPERTY, PropertyFlag_IS_THIS );

    m_name = _label;
    m_type = _type;
    m_components.set_owner( this );
}

size_t ASTNode::adjacent_slot_count(SlotFlags _flags )const
{
    return filter_adjacent_slots( _flags ).size();
}

const FunctionDescriptor* ASTNode::get_connected_function_type(const char* property_name) const
{
    const Slot* slot = find_slot_by_property_name( property_name, SlotFlag_INPUT );
    VERIFY(slot!= nullptr, "Unable to find input slot for this property name")
    const Slot* adjacent_slot = slot->first_adjacent();

    if ( adjacent_slot )
        if (adjacent_slot->node()->is_invokable() )
            return static_cast<const ASTFunctionNode*>(adjacent_slot->node())->get_func_type();

    return nullptr;
}

void ASTNode::set_name(const char *_label)
{
    m_name = _label;
    m_on_name_change.emit(this);
}

std::vector<ASTNodeComponent*> ASTNode::get_components()
{
    return m_components.get_all();
}

Slot* ASTNode::find_slot_by_property_name(const char* _property_name, SlotFlags _flags)
{
    return const_cast<Slot*>( const_cast<const ASTNode*>(this)->find_slot_by_property_name(_property_name, _flags));
}

const Slot* ASTNode::find_slot_by_property_name(const char* property_name, SlotFlags desired_way) const
{
    const Property* property = get_prop(property_name);
    if( property )
    {
        return find_slot_by_property( property, desired_way );
    }
    return nullptr;
}

const Property* ASTNode::get_prop(const char *_name) const
{
    return m_props.find_by_name( _name );
}

Property* ASTNode::get_prop(const char *_name)
{
    return m_props.find_by_name( _name );
}

Slot* ASTNode::find_slot(SlotFlags _flags)
{
    return const_cast<Slot*>( const_cast<const ASTNode*>(this)->find_slot(_flags));
}

const Slot* ASTNode::find_slot(SlotFlags _flags) const
{
    return find_slot_by_property(m_value, _flags );
}

Slot* ASTNode::find_slot_at(SlotFlags _flags, size_t _position)
{
    return const_cast<Slot*>( const_cast<const ASTNode*>(this)->find_slot_at(_flags, _position));
}

const Slot* ASTNode::find_slot_at(SlotFlags _flags, size_t _position) const
{
    for( const Slot* slot : m_slots )
    {
        if( slot->has_flags(_flags) && slot->position() == _position && slot->property() == m_value )
        {
            return slot;
        }
    }
    return nullptr;
}

Slot& ASTNode::slot_at(size_t pos)
{
    ASSERT(m_slots.size() < pos)
    return *m_slots[pos];
}

const Slot& ASTNode::slot_at(size_t pos) const
{
    ASSERT(m_slots.size() < pos)
    return *m_slots[pos];
}

std::vector<ASTNode*> ASTNode::outputs() const
{
    return filter_adjacent(SlotFlag_OUTPUT);
}

std::vector<ASTNode*> ASTNode::inputs() const
{
    return filter_adjacent(SlotFlag_INPUT);
}

std::vector<ASTNode*> ASTNode::predecessors() const
{
    return filter_adjacent(SlotFlag_PREV);
}

std::vector<ASTNode*> ASTNode::rchildren() const
{
    auto v = children();
    std::reverse( v.begin(), v.end() );
    return v;
}

std::vector<ASTNode*> ASTNode::children() const
{
    return filter_adjacent(SlotFlag_CHILD);
}

std::vector<ASTNode*> ASTNode::successors() const
{
    return filter_adjacent(SlotFlag_NEXT);
}

std::vector<ASTNode*> ASTNode::filter_adjacent(SlotFlags _flags ) const
{
    return ASTUtils::get_adjacent_nodes(this, _flags);
}

Slot* ASTNode::find_slot_by_property_type(SlotFlags flags, const TypeDescriptor* _type)
{
    for(Slot* slot : filter_slots( flags ) )
    {
        if( type::is_implicitly_convertible(slot->get_property()->get_type(), _type ) )
        {
            return slot;
        }
    }
    return nullptr;
}

Slot& ASTNode::nth_slot(size_t _n, SlotFlags _flags )
{
    size_t count = 0;
    for( auto& slot : m_slots )
    {
        if ( slot->has_flags(_flags) )
        {
            if( count == _n )
            {
                return *slot;
            }
            count++;
        }
    }
    VERIFY(false, "Not found")
}

Property* ASTNode::add_prop(const TypeDescriptor* _type, const char *_name, PropertyFlags _flags)
{
    return m_props.add(_type, _name, _flags);
}

Slot* ASTNode::add_slot(Property *_property, SlotFlags _flags, size_t _capacity, size_t _position)
{
    ASSERT( _property != nullptr )
    ASSERT(_property->owner() == this)
    Slot* slot = new Slot(this, _flags, _property, _capacity, _position);
    m_slots.push_back(slot);
    return slot;
}

ASTNode* ASTNode::find_parent() const
{
    if ( const Slot* parent_slot = find_slot( SlotFlag_PARENT ) )
    {
        Slot* adjacent_slot = parent_slot->first_adjacent();
        if(adjacent_slot == nullptr)
            return {};
        return adjacent_slot->node();
    }
    return {};
}

std::vector<Slot*> ASTNode::filter_adjacent_slots(SlotFlags _flags ) const
{
    std::vector<Slot*> result;

    for(Slot* slot : filter_slots(_flags))
        for( Slot* each : slot->adjacent() )
            result.push_back( each );

    return result;
}

bool ASTNode::has_input_connected(const Property* property ) const
{
    const Slot* slot = find_slot_by_property( property, SlotFlag_INPUT );
    return slot && slot->adjacent_count() > 0;
}

size_t ASTNode::slot_count(SlotFlags flags) const
{
    return filter_slots( flags ).size();
}

Slot* ASTNode::find_slot_by_property(const Property* property_id, SlotFlags _flags)
{
    return const_cast<Slot*>( const_cast<const ASTNode*>( this )->find_slot_by_property(property_id, _flags ) );
}

const Slot* ASTNode::find_slot_by_property(const Property* ptr, SlotFlags _flags) const
{
    //TODO: We may want to switch from a vector to an unordered/ordered map with pointer address as hash
    for(auto& slot : m_slots )
    {
        if( slot->has_flags(_flags) && slot->get_property() == ptr )
        {
            return slot;
        }
    }
    return nullptr;
}

Slot* ASTNode::find_adjacent_at(SlotFlags _flags, size_t _index ) const
{
    size_t cursor_pos{0};
    for (Slot* slot : m_slots)
    {
        // Skip any slot not compatible with given flags
        if( !slot->has_flags( _flags ) )
        {
            continue;
        }

        // if the position is in the range of this slot, we return the item
        size_t local_pos = (size_t)_index - cursor_pos;
        if ( local_pos < slot->adjacent_count() )
        {
            return slot->adjacent_at(local_pos);
        }
        // increase counter
        cursor_pos += slot->adjacent_count();
    }
    return nullptr;
}

std::vector<Slot*> ASTNode::filter_slots(SlotFlags _flags ) const
{
    std::vector<Slot*> result;
    for(auto& slot : m_slots)
    {
        if( slot && slot->has_flags(_flags) )
        {
            result.push_back(const_cast<Slot*>(slot));
        }
    }
    return result;
}

bool ASTNode::is_instruction() const
{
    if ( auto* slot = find_slot(SlotFlag_PREV) )
        if ( slot->adjacent_count() > 0 )
            return true;
    if ( auto* slot = find_slot(SlotFlag_NEXT) )
        if ( slot->adjacent_count() > 0 )
            return true;
    return false;
}

bool ASTNode::should_be_constrain_to_follow_output(const ASTNode* _output ) const
{
    const auto _outputs = outputs();
    return predecessors().empty() && !_outputs.empty() && _outputs.back() == _output;
}

bool ASTNode::can_be_instruction() const
{
    // TODO: handle case where a variable has inputs/outputs but not connected to the code flow
    return slot_count(SlotFlag_TYPE_CODEFLOW) > 0 && inputs().empty() && outputs().empty();
}

bool ASTNode::is_unary_operator() const
{
    if ( m_type == ASTNodeType_OPERATOR )
        if (static_cast<const ASTFunctionNode*>(this)->get_func_type()->get_arg_count() == 1 )
            return true;
    return false;
}

bool ASTNode::is_binary_operator() const
{
    if ( m_type == ASTNodeType_OPERATOR )
        if (static_cast<const ASTFunctionNode*>(this)->get_func_type()->get_arg_count() == 2 )
            return true;
    return false;
}

bool ASTNode::is_conditional() const
{
    switch ( m_type )
    {
        case ASTNodeType_BLOCK_FOR_LOOP:
        case ASTNodeType_BLOCK_WHILE_LOOP:
        case ASTNodeType_BLOCK_CONDITION:
            return true;
        default:
            return false;
    };
}

void ASTNode::set_suffix(const ASTToken& token)
{
    m_suffix = token;
}

const PropertyBag& ASTNode::props() const
{
    return m_props;
}

bool ASTNode::is_invokable() const
{
    return m_type == ASTNodeType_OPERATOR || m_type == ASTNodeType_FUNCTION;
}

Slot* ASTNode::value_out()
{
    return const_cast<Slot*>( find_slot_by_property(m_value, SlotFlag_OUTPUT ) );
}

const Slot* ASTNode::value_out() const
{
    return find_slot_by_property(m_value, SlotFlag_OUTPUT );
}

Slot* ASTNode::value_in()
{
    return const_cast<Slot*>( find_slot_by_property(m_value, SlotFlag_INPUT ) );
}

const Slot* ASTNode::value_in() const
{
    return find_slot_by_property(m_value, SlotFlag_INPUT );
}
