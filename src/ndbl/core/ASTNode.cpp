#include "ASTNode.h"

#include <algorithm> // for std::find

#include "ASTScope.h"
#include "Graph.h"
#include "ASTUtils.h"

using namespace ndbl;
using namespace tools;

ASTNode::ASTNode()
: ASTNode(ASTNodeType_NULL)
{
}

ASTNode::ASTNode(ASTNodeType type)
: m_component_collection(this)
, m_adjacent_nodes_cache(this)
, m_type(type)
{
    construct_union_data();
}

ASTNode::~ASTNode()
{
    destroy_union_data();
    assert(m_slots.empty());
    assert(m_properties_by_name.empty());
    assert(m_properties.empty());
    assert(m_component_collection.components().empty());
}

void ASTNode::construct_union_data()
{
    switch (m_type)
    {
        case ASTNodeType_IF_ELSE:    [[fallthrough]];     
        case ASTNodeType_WHILE_LOOP: [[fallthrough]];
        case ASTNodeType_FOR_LOOP:
        {
            new (&m_switch_behavior_data) decltype(m_switch_behavior_data)();
            break;
        }

        case ASTNodeType_LITERAL:
        {
            new (&m_literal_data) decltype(m_literal_data)(this);
            break;
        }

        case ASTNodeType_OPERATOR: [[fallthrough]];
        case ASTNodeType_FUNCTION:
        {
            new (&m_invokable_data) decltype(m_invokable_data)(this);
            break;
        }

        case ASTNodeType_VARIABLE_REF:
        {
            new (&m_variable_ref_data) decltype(m_variable_ref_data)(this);
            break;
        }

        case ASTNodeType_VARIABLE:
        {
            new (&m_variable_data) decltype(m_variable_data)(this);
            break;
        }

        case ASTNodeType_ROOT:              [[fallthrough]];
        case ASTNodeType_SCOPE:             [[fallthrough]];
        case ASTNodeType_EMPTY_INSTRUCTION: [[fallthrough]];
        case ASTNodeType_NULL:
            // Those types do not have a dedicated data struct in the union
            break;

        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            TOOLS_UNREACHABLE();
    }
}

void ASTNode::destroy_union_data()
{
    switch (m_type)
    {
        case ASTNodeType_IF_ELSE:    [[fallthrough]];
        case ASTNodeType_WHILE_LOOP: [[fallthrough]];
        case ASTNodeType_FOR_LOOP:
        {
            m_switch_behavior_data.~SwitchBehaviorData();
            break;
        }

        case ASTNodeType_LITERAL:
        {
            m_literal_data.~LiteralData();
            break;
        }

        case ASTNodeType_OPERATOR: [[fallthrough]];
        case ASTNodeType_FUNCTION:
        {
            m_invokable_data.~InvokableData();
            break;
        }

        case ASTNodeType_VARIABLE_REF:
        {
            m_variable_ref_data.~VariableRefData();
            break;
        }

        case ASTNodeType_VARIABLE:
        {
            m_variable_data.~VariableData();
            break;
        }

        case ASTNodeType_ROOT:              [[fallthrough]];
        case ASTNodeType_SCOPE:             [[fallthrough]];
        case ASTNodeType_EMPTY_INSTRUCTION: [[fallthrough]];
        case ASTNodeType_NULL:
            // Those types do not have a dedicated data struct in the union
            break;

        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            TOOLS_UNREACHABLE();
    }
}

void ASTNode::init(const std::string& label)
{
    VERIFY(m_is_initialized == false, "You cannot initialize twice");

    m_value = add_prop<any>(DEFAULT_PROPERTY, PropertyFlag_IS_NODE_VALUE );
    set_name(label);

    m_is_initialized = true;
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
            return adjacent_slot->node->invokable_data().get_func_type();

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

bool ASTNode::has_switch_behavior() const
{
    switch (m_type)
    {
    case ASTNodeType_FOR_LOOP:
    case ASTNodeType_IF_ELSE:
    case ASTNodeType_WHILE_LOOP:
        return true;
    
    default:
        return false;
    }
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

    if ( has_internal_scope() ) internal_scope()->reset_parent( scope );
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

ASTNode::InvokableData::InvokableData(ASTNode* node)
: m_node(node)
{
}

void ASTNode::InvokableData::init(const tools::FunctionDescriptor& _func_type )
{
    VERIFY( m_node && m_node->value() != nullptr, "Node does not seems to be initialized");

    m_func_type = _func_type;
    m_identifier_token = {
            ASTToken_t::identifier,
            _func_type.get_identifier()
    };
    m_argument_slot.resize(_func_type.arg_count());
    m_argument_props.resize(_func_type.arg_count());

    switch ( m_node->m_type )
    {
        case ASTNodeType_OPERATOR:
            m_node->set_name(_func_type.get_identifier());
            break;
        case ASTNodeType_FUNCTION:
        {
            const std::string& id   = _func_type.get_identifier();
            std::string label       = id; // We add dynamically the brackets (see NodeView)
            std::string short_label = id.substr(0, 2) + "..";
            m_node->set_name(label.c_str());
            break;
        }
        default:
            VERIFY(false, "Type not allowed");
    }

    // Create a result/value
    auto value = m_node->value();
    value->set_type(_func_type.return_type() );

    m_node->add_slot(value, SlotFlag_OUTPUT );
    m_node->add_slot(value, SlotFlag_FLOW_OUT , 1);
    m_node->add_slot(value, SlotFlag_FLOW_IN );

    // Create arguments
    if (m_node->m_type == ASTNodeType_OPERATOR )
    {
        VERIFY(_func_type.arg_count() >= 1, "An operator must have one argument minimum");
        VERIFY(_func_type.arg_count() <= 2, "An operator cannot have more than 2 arguments");
    }

    for (size_t i = 0; i < _func_type.arg_count(); i++ )
    {
        const FuncArg& arg  = _func_type.arg_at(i);

        const char* name;
        // TODO: this could be done in the NodeView instead...
        if (m_node->m_type == ASTNodeType_OPERATOR )
        {
            if ( i == 0 )
                name = LEFT_VALUE_PROPERTY ;
            else if ( i == 1 )
                name = RIGHT_VALUE_PROPERTY;
        }
        else
        {
            name = arg.name.c_str();
        }

        ASTNodeProperty* property  = m_node->add_prop(arg.type, name );

        if ( arg.pass_by_ref )
            property->set_flags(PropertyFlag_IS_REF);

        m_argument_slot[i]  = m_node->add_slot(property, SlotFlag_INPUT, 1);
        m_argument_props[i] = property;
    }
}

ASTNode::VariableData::VariableData(ASTNode* node)
: m_owner_node(node)
{
}

void ASTNode::VariableData::init(const tools::TypeDescriptor* _type, const char* _identifier)
{
    VERIFY( m_owner_node && m_owner_node->value() != nullptr, "Node does not seems to be initialized");

    auto value = m_owner_node->value();

    // Init identifier property
    value->set_type(_type);
    value->set_token({ASTToken_t::identifier});
    value->token().word_replace(_identifier); // might come from std::string::c_str()

    // Init Slots
    m_owner_node->add_slot(value, SlotFlag_INPUT, 1); // to connect an initialization expression
    m_owner_node->add_slot(value, SlotFlag_FLOW_OUT, 1);
    m_owner_node->add_slot(value, SlotFlag_FLOW_IN);

    m_as_declaration_slot = m_owner_node->add_slot(value, SlotFlag_OUTPUT, 1); // as declaration
    m_as_reference_slot   = m_owner_node->add_slot(value, SlotFlag_OUTPUT); // as reference
}


ASTNode::VariableRefData::VariableRefData(ASTNode* node)
: m_owner_node(node)
{
}

ASTNode::VariableRefData::~VariableRefData()
{
    clear_variable();
}

void ASTNode::VariableRefData::init()
{
    VERIFY( m_owner_node && m_owner_node->value() != nullptr, "Node does not seems to be initialized");

    auto value = m_owner_node->value();

    // Init identifier property
    value->set_type(tools::type::any());
    value->set_token({ASTToken_t::identifier});

    // Init Slots
    m_owner_node->add_slot(value, SlotFlag_FLOW_OUT, 1);
    m_owner_node->add_slot(value, SlotFlag_FLOW_IN);
    m_owner_node->add_slot(value, SlotFlag_INPUT   , 1);
    m_owner_node->add_slot(value, SlotFlag_OUTPUT  , 1); // ref can be connected once
}

void ASTNode::VariableRefData::set_variable(ASTNode* variable_node)
{
    VERIFY( m_variable_node == nullptr, "Can't call twice");

    m_variable_node = variable_node;

    auto& variable_data = m_variable_node->variable_data();
    m_owner_node->value()->set_type( variable_data.get_type() );
    m_owner_node->value()->token().word_replace( variable_data.get_identifier().c_str() );

    // bind signals
    m_variable_node->signal_name_change.connect<&VariableRefData::handle_name_change>(this);
    m_variable_node->signal_shutdown.connect<&VariableRefData::clear_variable>(this);
}

void ASTNode::VariableRefData::clear_variable()
{
    if ( m_variable_node == nullptr )
        return;

    // unbind signals
    m_variable_node->signal_name_change.disconnect();
    m_variable_node->signal_shutdown.disconnect();

    m_variable_node = nullptr;
}

const ASTToken& ASTNode::VariableRefData::get_identifier_token() const
{
    return m_owner_node->value()->token(); // when parsed, this token may be a bit different from m_variable's (trailing ignored characters)
}

void ASTNode::VariableRefData::handle_name_change(const std::string& name)
{
    m_owner_node->value()->token().word_replace( name.c_str() );
}

ASTNode::LiteralData::LiteralData(ASTNode* node)
: m_node(node)
{
}

void ASTNode::LiteralData::init(const TypeDescriptor* _type)
{
    VERIFY( m_node && m_node->value() != nullptr, "Node does not seems to be initialized");

    auto value = m_node->value();

    value->set_type(_type);

    m_node->add_slot(value, SlotFlag_FLOW_OUT , 1);
    m_node->add_slot(value, SlotFlag_FLOW_IN);
    m_node->add_slot(value, SlotFlag_OUTPUT   , 1);
}


void ASTNode::switch_behavior_create_branches(size_t branch_count)
{
    VERIFY( 1 < branch_count && branch_count <= SwitchBehaviorData::BRANCH_MAX, "branch_count is out of range");
    VERIFY( has_switch_behavior(), "Node does not have a switch behavior" );


    m_switch_behavior_data.m_branch_count = branch_count;

    add_slot(value(), SlotFlag_FLOW_IN);      // accepts N inputs
    add_slot(value(), SlotFlag_FLOW_OUT , 1); // accepts 0 or 1 output

    // add 1 slot per branch
    for(size_t branch = 0; branch < branch_count; ++branch )
    {
        m_switch_behavior_data.m_branch_slot[branch] = add_slot(value(), SlotFlag_FLOW_ENTER, 1, branch);
    }

    // add 1 condition per branch except for the default branch
    for(size_t branch = 1; branch < branch_count; ++branch )
    {
        auto condition_property = add_prop<tools::any>(CONDITION_PROPERTY);
        m_switch_behavior_data.m_condition_in[branch-1]  = add_slot(condition_property, SlotFlag_INPUT, 1, branch);
    }
}