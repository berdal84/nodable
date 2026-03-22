#include "ASTNode.h"

#include <algorithm> // for std::find

#include "ASTScope.h"
#include "Graph.h"
#include "ASTUtils.h"
#include "ASTFunctionCall.h"

using namespace ndbl;
using namespace tools;

ASTNode::~ASTNode()
{
    assert(m_slots.empty());
    assert(m_properties_by_name.empty());
    assert(m_properties.empty());
    assert(m_component_collection.components().empty());
}

void ASTNode::init(ASTNodeType type, const std::string& label)
{
    m_value = add_prop<any>(DEFAULT_PROPERTY, PropertyFlag_IS_NODE_VALUE );
    set_name(label);
    m_type  = type;

    switch (type)
    {
        case ASTNodeType_IF_ELSE:            
            branch_prefix = {ASTToken_t::keyword_if};
            branch_init(2);
            break;

        case ASTNodeType_WHILE_LOOP:
            branch_prefix = {ASTToken_t::keyword_while};
            branch_init(2);
            break;

        case ASTNodeType_FOR_LOOP:
            branch_prefix = {ASTToken_t::keyword_for};

            // add initialization property and slot
            auto* init_prop = add_prop<any>(INITIALIZATION_PROPERTY );
            m_initialization_slot = add_slot(init_prop, SlotFlag_INPUT, 1);

            // add conditional-related properties and slots
            branch_init(2);

            // add iteration property and slot
            auto iter_prop = add_prop<any>(ITERATION_PROPERTY );
            m_iteration_slot = add_slot(iter_prop, SlotFlag_INPUT, 1);
            break;
    }
}

void ASTNode::shutdown()
{
    while( !m_properties.empty() )
    {
        size_t erased_count = m_properties_by_name.erase( m_properties.back()->name() );
        ASSERT(erased_count==1);
        delete m_properties.back();
        m_properties.pop_back();
    }

    while( !m_slots.empty() )
    {
        delete m_slots.back();
        m_slots.pop_back();
    }

    m_component_collection.shutdown();
    signal_shutdown.emit();
}

const FunctionDescriptor* ASTNode::get_connected_function_type(const char* property_name) const
{
    const ASTNodeSlot* slot = find_slot_by_property_name(property_name, SlotFlag_INPUT );
    VERIFY(slot!= nullptr, "Unable to find input slot for this property name");
    const ASTNodeSlot* adjacent_slot = slot->first_adjacent();

    if ( adjacent_slot )
        if (adjacent_slot->node->is_invokable() )
            return &static_cast<const ASTFunctionCall*>(adjacent_slot->node)->get_func_type();

    return nullptr;
}

const ASTNodeSlot* ASTNode::find_slot_by_property_name(const char* property_name, SlotFlags desired_way) const
{
    const ASTNodeProperty* property = get_prop(property_name);
    if( property )
    {
        return find_slot_by_property( property, desired_way );
    }
    return nullptr;
}

const ASTNodeSlot* ASTNode::find_slot_at(SlotFlags _flags, size_t _position) const
{
    for( const ASTNodeSlot* slot : m_slots )
    {
        if( slot->has_flags(_flags) && slot->position == _position && slot->property == m_value )
        {
            return slot;
        }
    }
    return nullptr;
}

ASTNodeSlot* ASTNode::find_slot_by_property_type(SlotFlags flags, const TypeDescriptor* _type) const
{
    for(ASTNodeSlot* slot : filter_slots(flags ) )
    {
        if( type::is_implicitly_convertible(slot->property->get_type(), _type ) )
        {
            return slot;
        }
    }
    return nullptr;
}

void ASTNode::_handle_slot_change(ASTNodeSlot::Event event, ASTNodeSlot* slot)
{
    this->m_adjacent_nodes_cache.set_dirty();
}

ASTNodeSlot* ASTNode::add_slot(ASTNodeProperty* property, SlotFlags flags, size_t capacity, size_t position)
{
    ASSERT( property != nullptr );
    ASSERT( property->node() == this );
    if ( (flags & SlotFlag_FLOW_OUT) == SlotFlag_FLOW_OUT)
    {
        VERIFY( capacity == 1, "SlotFlag_FLOW_OUT can only have a capacity of 1" );
    }

    ASTNodeSlot* slot = new ASTNodeSlot(flags, capacity, position);
    slot->node     = this;
    slot->property = property;

    m_slots.push_back(slot);

    // Insert in "prop to slot" index
    // TODO: use a vector of vector? (having same size_t indexes as m_properties vector => O(1) access )
    m_slots_by_property[property].push_back(slot);

    // listen to events to clear cache
    slot->signal_change.connect<&ASTNode::_handle_slot_change>(this);

    return slot;
}

std::vector<ASTNodeSlot*> ASTNode::filter_adjacent_slots(SlotFlags _flags ) const
{
    std::vector<ASTNodeSlot*> result;

    for(ASTNodeSlot* slot : filter_slots(_flags))
        for( ASTNodeSlot* each : slot->adjacent() )
            result.push_back( each );

    return result;
}

bool ASTNode::has_input_connected(const ASTNodeProperty* property ) const
{
    const ASTNodeSlot* slot = find_slot_by_property(property, SlotFlag_INPUT );
    return slot && slot->adjacent_count() > 0;
}

const ASTNodeSlot* ASTNode::find_slot_by_property(const ASTNodeProperty* prop, SlotFlags flags) const
{
    auto it = m_slots_by_property.find(prop);
    if ( it != m_slots_by_property.end() )
        for( ASTNodeSlot* slot : it->second )
            if( slot->has_flags(flags) )
                return slot;
    return nullptr;
}

ASTNodeSlot* ASTNode::find_adjacent_at(SlotFlags _flags, size_t _index ) const
{
    size_t cursor_pos{0};
    for (ASTNodeSlot* slot : m_slots)
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

std::vector<ASTNodeSlot*> ASTNode::filter_slots(SlotFlags flags) const
{
    const auto if_has_flags = [flags](const ASTNodeSlot* _slot)
    {
        ASSERT_DEBUG_ONLY(_slot != nullptr);
        return _slot->has_flags(flags);
    };
    return filter_slots(if_has_flags);
}

std::vector<ASTNodeSlot*> ASTNode::filter_slots(const std::function<bool(const ASTNodeSlot*)>& predicate) const
{
    std::vector<ASTNodeSlot*> result;
    std::copy_if( m_slots.begin(), m_slots.end(), std::back_inserter(result), predicate);
    return result;
}

void ASTNode::set_suffix(const ASTToken& token)
{
    m_suffix = token;
}

bool ASTNode::is_invokable() const
{
    return m_type == ASTNodeType_OPERATOR || m_type == ASTNodeType_FUNCTION;
}

ASTNodeSlot* ASTNode::value_out()
{
    return const_cast<ASTNodeSlot*>( find_slot_by_property(m_value, SlotFlag_OUTPUT ) );
}

const ASTNodeSlot* ASTNode::value_out() const
{
    return find_slot_by_property(m_value, SlotFlag_OUTPUT );
}

ASTNodeSlot* ASTNode::value_in()
{
    return const_cast<ASTNodeSlot*>( find_slot_by_property(m_value, SlotFlag_INPUT ) );
}

const ASTNodeSlot* ASTNode::value_in() const
{
    return find_slot_by_property(m_value, SlotFlag_INPUT );
}

ASTNodeSlot* ASTNode::flow_enter()
{
    auto* const_this = const_cast<const ASTNode*>(this);
    return const_cast<ASTNodeSlot*>( const_this->flow_enter());
}

const ASTNodeSlot* ASTNode::flow_enter() const
{
    auto it = m_slots_by_property.find(m_value);
    if ( it != m_slots_by_property.end() )
    {
        const auto& [_, slots] = *it;
        for( ASTNodeSlot* slot : slots )
            if( slot->has_flags(SlotFlag_FLOW_ENTER) )
                return slot;
    }
    return nullptr;
}

ASTNodeSlot* ASTNode::flow_out()
{
    auto* const_this = const_cast<const ASTNode*>(this);
    return const_cast<ASTNodeSlot*>( const_this->flow_out());
}

const ASTNodeSlot* ASTNode::flow_out() const
{
    auto it = m_slots_by_property.find(m_value);
    if ( it != m_slots_by_property.end() )
    {
        const auto& [_, slots] = *it;
        for( ASTNodeSlot* slot : slots )
            if( slot->has_flags(SlotFlag_FLOW_OUT) && !slot->has_flags(SlotFlag_IS_INTERNAL) ) // branches are specific flow_out, we don't want to grab them here
                return slot;
    }
    return nullptr;
}

ASTNodeSlot* ASTNode::flow_in()
{
    return const_cast<ASTNodeSlot*>( find_slot_by_property(m_value, SlotFlag_FLOW_IN ) );
}

const ASTNodeSlot* ASTNode::flow_in() const
{
    return find_slot_by_property(m_value, SlotFlag_FLOW_IN );
}

bool ASTNode::update()
{
    //
    // some code here
    //

    clear_flags(ASTNodeFlag_IS_DIRTY);

    return true;
}

const std::vector<ASTNode*>& ASTNode::AdjacentNodesCache::get(SlotFlags flags ) const
{
    if ( _cache.find(flags) == _cache.end() )
    {
        auto _this = const_cast<AdjacentNodesCache*>(this);
        _this->_cache.insert_or_assign(flags, ASTUtils::get_adjacent_nodes(_node, flags ) );
    }

    return _cache.at(flags);
}

void ASTNode::init_internal_scope()
{
    VERIFY( m_internal_scope == nullptr, "Can't call init_internal_scope() more than once");
    VERIFY( m_parent_scope == nullptr, "Must be initialized prior to reset_parent()");

    auto* scope = this->components()->create<ASTScope>();
    scope->set_name("Internal Scope");

    m_internal_scope = scope;
}

bool ASTNode::has_flow_adjacent() const
{
    return !flow_inputs().empty() || !flow_outputs().empty();
}

bool ASTNode::is_expression() const
{
    return !inputs().empty();
}

void ASTNode::reset_scope(ASTScope* scope)
{
#ifdef TOOLS_DEBUG
    if ( scope == nullptr )
        VERIFY( m_flags & ASTNodeFlag_WAS_IN_A_SCOPE_ONCE, "This node never been in a scope, why would you reset it to nullptr? (that's the default value)")
#endif
    m_flags |= ASTNodeFlag_WAS_IN_A_SCOPE_ONCE;
    m_parent_scope = scope;

    if ( m_internal_scope )
        m_internal_scope->reset_parent( scope );
}

bool ASTNode::has_prop(const char* _name) const
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

ASTNodeProperty* ASTNode::add_prop(const TypeDescriptor* type, const char* name, PropertyFlags flags )
{
    // guards
    VERIFY(!has_prop(name), "Property name already used");

    // create
    auto* new_property = new ASTNodeProperty; // TODO: use a static-sized array with a given limit (ex: 10 props)
    new_property->init(this, type, flags, name);

    // register / index
    m_properties.push_back(new_property);
    m_properties_by_name.insert({new_property->name(), new_property});

    return new_property;
}

const ASTNodeProperty* ASTNode::find_first_prop(PropertyFlags _flags, const TypeDescriptor *_type) const
{
    auto filter = [_flags, _type](const std::pair<const std::string, ASTNodeProperty*>& pair) -> bool
    {
        auto* property = pair.second;
        return type::is_implicitly_convertible(property->get_type(), _type)
               && ( property->has_flags( _flags ) );
    };

    auto found = std::find_if(m_properties_by_name.begin(), m_properties_by_name.end(), filter );
    if ( found != m_properties_by_name.end())
        return found->second;
    return nullptr;
}

const ASTNodeProperty* ASTNode::find_prop_by_name(const char* name) const
{
    for(auto& [_name, property] : m_properties_by_name )
    {
        if( _name == name)
            return property;
    }
    ASSERT(false);
    return nullptr;
}

void ASTNode::branch_init(size_t branch_count)
{
    VERIFY( 1 < branch_count && branch_count <= BRANCH_MAX, "branch_count is out of range");

    m_branch_count = branch_count;

    add_slot(value(), SlotFlag_FLOW_IN);      // accepts N inputs
    add_slot(value(), SlotFlag_FLOW_OUT , 1); // accepts 0 or 1 output

    init_internal_scope();

    // add 1 slot per branch
    for(size_t branch = 0; branch < branch_count; ++branch )
    {
        m_branch_slot[branch] = add_slot(value(), SlotFlag_FLOW_ENTER, 1, branch);
    }

    // add 1 condition per branch except for the default branch
    for(size_t branch = 1; branch < branch_count; ++branch )
    {
        auto condition_property  = add_prop<tools::any>(CONDITION_PROPERTY);
        m_condition_in[branch-1] = add_slot(condition_property, SlotFlag_INPUT, 1, branch);
    }
}