#include "Node.h"

#include <algorithm> // for std::find
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "core/Constants.h"
#include "Scope.h"
#include "Graph.h"

using namespace ndbl;
using namespace tools;

Node::Node()
: Node(Node_Type_NULL)
{
}

Node::Node(Node_Type type)
: m_component_collection(this)
, m_adjacent_nodes_cache(this)
, m_type(type)
{
    construct_union_data();
}

Node::~Node()
{
    destroy_union_data();
    assert(m_slots.empty());
    assert(m_properties_by_name.empty());
    assert(m_properties.empty());
    assert(m_component_collection.components().empty());
}

void Node::construct_union_data()
{
    switch (m_type)
    {
        case Node_Type_IF_ELSE:    [[fallthrough]];     
        case Node_Type_WHILE_LOOP: [[fallthrough]];
        case Node_Type_FOR_LOOP:
        {
            new (&m_switch_behavior_data) decltype(m_switch_behavior_data)();
            break;
        }

        case Node_Type_LITERAL:
        {
            new (&m_literal_data) decltype(m_literal_data)(this);
            break;
        }

        case Node_Type_OPERATOR: [[fallthrough]];
        case Node_Type_FUNCTION:
        {
            new (&m_invokable_data) decltype(m_invokable_data)(this);
            break;
        }

        case Node_Type_VARIABLE_REF:
        {
            new (&m_variable_ref_data) decltype(m_variable_ref_data)(this);
            break;
        }

        case Node_Type_VARIABLE:
        {
            new (&m_variable_data) decltype(m_variable_data)(this);
            break;
        }

        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             [[fallthrough]];
        case Node_Type_EMPTY_INSTRUCTION: [[fallthrough]];
        case Node_Type_NULL:
            // Those types do not have a dedicated data struct in the union
            break;

        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            TOOLS_UNREACHABLE();
    }
}

void Node::destroy_union_data()
{
    switch (m_type)
    {
        case Node_Type_IF_ELSE:    [[fallthrough]];
        case Node_Type_WHILE_LOOP: [[fallthrough]];
        case Node_Type_FOR_LOOP:
        {
            m_switch_behavior_data.~Switch_Behavior_State();
            break;
        }

        case Node_Type_LITERAL:
        {
            m_literal_data.~Literal_State();
            break;
        }

        case Node_Type_OPERATOR: [[fallthrough]];
        case Node_Type_FUNCTION:
        {
            m_invokable_data.~Invokable_State();
            break;
        }

        case Node_Type_VARIABLE_REF:
        {
            m_variable_ref_data.~Variable_Ref_State();
            break;
        }

        case Node_Type_VARIABLE:
        {
            m_variable_data.~Variable_State();
            break;
        }

        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             [[fallthrough]];
        case Node_Type_EMPTY_INSTRUCTION: [[fallthrough]];
        case Node_Type_NULL:
            // Those types do not have a dedicated data struct in the union
            break;

        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            TOOLS_UNREACHABLE();
    }
}

void Node::init(const std::string& label)
{
    VERIFY(m_is_initialized == false, "You cannot initialize twice");

    m_value = add_prop<any>(DEFAULT_PROPERTY, Node_Property_Flag_IS_NODE_VALUE );
    set_name(label);

    m_is_initialized = true;
}

void Node::shutdown()
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

const Function_Descriptor* Node::get_connected_function_type(const char* property_name) const
{
    const Node_Slot* slot = find_slot_by_property_name(property_name, Node_Slot_Flag_INPUT );
    VERIFY(slot!= nullptr, "Unable to find input slot for this property name");
    const Node_Slot* adjacent_slot = slot->first_adjacent();

    if ( adjacent_slot )
        if (adjacent_slot->node->is_invokable() )
            return adjacent_slot->node->invokable_data().get_func_type();

    return nullptr;
}

const Node_Slot* Node::find_slot_by_property_name(const char* property_name, Node_Slot_Flags desired_way) const
{
    const Node_Property* property = get_prop(property_name);
    if( property )
    {
        return find_slot_by_property( property, desired_way );
    }
    return nullptr;
}

const Node_Slot* Node::find_slot_at(Node_Slot_Flags _flags, size_t _position) const
{
    for( const Node_Slot* slot : m_slots )
    {
        if( slot->has_flags(_flags) && slot->position == _position && slot->property == m_value )
        {
            return slot;
        }
    }
    return nullptr;
}

Node_Slot* Node::find_slot_by_property_type(Node_Slot_Flags flags, const Type_Descriptor* _type) const
{
    for(Node_Slot* slot : filter_slots(flags ) )
    {
        if( type::is_implicitly_convertible(slot->property->get_type(), _type ) )
        {
            return slot;
        }
    }
    return nullptr;
}

void Node::_handle_slot_change(Node_Slot::Event event, Node_Slot* slot)
{
    this->m_adjacent_nodes_cache.set_dirty();
}

Node_Slot* Node::add_slot(Node_Property* property, Node_Slot_Flags flags, size_t capacity, size_t position)
{
    ASSERT( property != nullptr );
    ASSERT( property->node() == this );
    if ( (flags & Node_Slot_Flag_FLOW_OUT) == Node_Slot_Flag_FLOW_OUT)
    {
        VERIFY( capacity == 1, "Node_Slot_Flag_FLOW_OUT can only have a capacity of 1" );
    }

    Node_Slot* slot = new Node_Slot(flags, capacity, position);
    slot->node     = this;
    slot->property = property;

    m_slots.push_back(slot);

    // Insert in "prop to slot" index
    // TODO: use a vector of vector? (having same size_t indexes as m_properties vector => O(1) access )
    m_slots_by_property[property].push_back(slot);

    // listen to events to clear cache
    slot->signal_change.connect<&Node::_handle_slot_change>(this);

    return slot;
}

std::vector<Node_Slot*> Node::filter_adjacent_slots(Node_Slot_Flags _flags ) const
{
    std::vector<Node_Slot*> result;

    for(Node_Slot* slot : filter_slots(_flags))
        for( Node_Slot* each : slot->adjacent() )
            result.push_back( each );

    return result;
}

bool Node::has_input_connected(const Node_Property* property ) const
{
    const Node_Slot* slot = find_slot_by_property(property, Node_Slot_Flag_INPUT );
    return slot && slot->adjacent_count() > 0;
}

const Node_Slot* Node::find_slot_by_property(const Node_Property* prop, Node_Slot_Flags flags) const
{
    auto it = m_slots_by_property.find(prop);
    if ( it != m_slots_by_property.end() )
        for( Node_Slot* slot : it->second )
            if( slot->has_flags(flags) )
                return slot;
    return nullptr;
}

Node_Slot* Node::find_adjacent_at(Node_Slot_Flags _flags, size_t _index ) const
{
    size_t cursor_pos{0};
    for (Node_Slot* slot : m_slots)
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

std::vector<Node_Slot*> Node::filter_slots(Node_Slot_Flags flags) const
{
    const auto if_has_flags = [flags](const Node_Slot* _slot)
    {
        ASSERT_DEBUG_ONLY(_slot != nullptr);
        return _slot->has_flags(flags);
    };
    return filter_slots(if_has_flags);
}

std::vector<Node_Slot*> Node::filter_slots(const std::function<bool(const Node_Slot*)>& predicate) const
{
    std::vector<Node_Slot*> result;
    std::copy_if( m_slots.begin(), m_slots.end(), std::back_inserter(result), predicate);
    return result;
}

void Node::set_suffix(const Token& token)
{
    m_suffix = token;
}

Node_Slot* Node::value_out()
{
    return const_cast<Node_Slot*>( find_slot_by_property(m_value, Node_Slot_Flag_OUTPUT ) );
}

const Node_Slot* Node::value_out() const
{
    return find_slot_by_property(m_value, Node_Slot_Flag_OUTPUT );
}

Node_Slot* Node::value_in()
{
    return const_cast<Node_Slot*>( find_slot_by_property(m_value, Node_Slot_Flag_INPUT ) );
}

const Node_Slot* Node::value_in() const
{
    return find_slot_by_property(m_value, Node_Slot_Flag_INPUT );
}

Node_Slot* Node::flow_enter()
{
    auto* const_this = const_cast<const Node*>(this);
    return const_cast<Node_Slot*>( const_this->flow_enter());
}

const Node_Slot* Node::flow_enter() const
{
    auto it = m_slots_by_property.find(m_value);
    if ( it != m_slots_by_property.end() )
    {
        const auto& [_, slots] = *it;
        for( Node_Slot* slot : slots )
            if( slot->has_flags(Node_Slot_Flag_FLOW_ENTER) )
                return slot;
    }
    return nullptr;
}

Node_Slot* Node::flow_out()
{
    auto* const_this = const_cast<const Node*>(this);
    return const_cast<Node_Slot*>( const_this->flow_out());
}

const Node_Slot* Node::flow_out() const
{
    auto it = m_slots_by_property.find(m_value);
    if ( it != m_slots_by_property.end() )
    {
        const auto& [_, slots] = *it;
        for( Node_Slot* slot : slots )
            if( slot->has_flags(Node_Slot_Flag_FLOW_OUT) && !slot->has_flags(Node_Slot_Flag_IS_INTERNAL) ) // branches are specific flow_out, we don't want to grab them here
                return slot;
    }
    return nullptr;
}

Node_Slot* Node::flow_in()
{
    return const_cast<Node_Slot*>( find_slot_by_property(m_value, Node_Slot_Flag_FLOW_IN ) );
}

const Node_Slot* Node::flow_in() const
{
    return find_slot_by_property(m_value, Node_Slot_Flag_FLOW_IN );
}

bool Node::update()
{
    //
    // some code here
    //

    clear_flags(Node_Flag_IS_DIRTY);

    return true;
}

const std::vector<Node*>& Node::Adjacent_Nodes_Cache::get(Node_Slot_Flags flags ) const
{
    if ( _cache.find(flags) == _cache.end() )
    {
        auto _this = const_cast<Adjacent_Nodes_Cache*>(this);
        _this->_cache.insert_or_assign(flags, node_get_adjacent_nodes(_node, flags ) );
    }

    return _cache.at(flags);
}

void Node::init_internal_scope()
{
    VERIFY( m_internal_scope == nullptr, "Can't call init_internal_scope() more than once");
    VERIFY( m_parent_scope == nullptr, "Must be initialized prior to reset_parent()");

    auto* scope = this->components()->create<Scope>();
    scope->set_name("Internal Scope");

    m_internal_scope = scope;
}

bool Node::has_flow_adjacent() const
{
    return !flow_inputs().empty() || !flow_outputs().empty();
}

bool Node::has_switch_behavior() const
{
    switch (m_type)
    {
    case Node_Type_FOR_LOOP:
    case Node_Type_IF_ELSE:
    case Node_Type_WHILE_LOOP:
        return true;
    
    default:
        return false;
    }
}

bool Node::is_expression() const
{
    return !inputs().empty();
}

void Node::reset_scope(Scope* scope)
{
#ifdef TOOLS_DEBUG
    if ( scope == nullptr )
        VERIFY( m_flags & Node_Flag_WAS_IN_A_SCOPE_ONCE, "This node never been in a scope, why would you reset it to nullptr? (that's the default value)")
#endif
    m_flags |= Node_Flag_WAS_IN_A_SCOPE_ONCE;
    m_parent_scope = scope;

    if ( has_internal_scope() ) internal_scope()->reset_parent( scope );
}

bool Node::has_prop(const char* _name) const
{
    return m_properties_by_name.find(_name) != m_properties_by_name.end();
}

Node_Property* Node::add_prop(const Type_Descriptor* type, const char* name, Node_Property_Flags flags )
{
    // guards
    VERIFY(!has_prop(name), "Property name already used");

    // create
    auto* new_property = new Node_Property; // TODO: use a static-sized array with a given limit (ex: 10 props)
    new_property->init(this, type, flags, name);

    // register / index
    m_properties.push_back(new_property);
    m_properties_by_name.insert({new_property->name(), new_property});

    return new_property;
}

const Node_Property* Node::find_first_prop(Node_Property_Flags _flags, const Type_Descriptor *_type) const
{
    auto filter = [_flags, _type](const std::pair<const std::string, Node_Property*>& pair) -> bool
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

const Node_Property* Node::find_prop_by_name(const char* name) const
{
    for(auto& [_name, property] : m_properties_by_name )
    {
        if( _name == name)
            return property;
    }
    ASSERT(false);
    return nullptr;
}

Node::Invokable_State::Invokable_State(Node* node)
: m_node(node)
{
}

void Node::Invokable_State::init(const tools::Function_Descriptor& _func_type )
{
    VERIFY( m_node && m_node->value() != nullptr, "Node does not seems to be initialized");

    m_func_type = _func_type;
    m_identifier_token = {
            Token_Type::identifier,
            _func_type.get_identifier()
    };
    m_argument_slot.resize(_func_type.arg_count());
    m_argument_props.resize(_func_type.arg_count());

    switch ( m_node->m_type )
    {
        case Node_Type_OPERATOR:
            m_node->set_name(_func_type.get_identifier());
            break;
        case Node_Type_FUNCTION:
        {
            const std::string& id   = _func_type.get_identifier();
            std::string label       = id; // We add dynamically the brackets (see Node_View)
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

    m_node->add_slot(value, Node_Slot_Flag_OUTPUT );
    m_node->add_slot(value, Node_Slot_Flag_FLOW_OUT , 1);
    m_node->add_slot(value, Node_Slot_Flag_FLOW_IN );

    // Create arguments
    if (m_node->m_type == Node_Type_OPERATOR )
    {
        VERIFY(_func_type.arg_count() >= 1, "An operator must have one argument minimum");
        VERIFY(_func_type.arg_count() <= 2, "An operator cannot have more than 2 arguments");
    }

    for (size_t i = 0; i < _func_type.arg_count(); i++ )
    {
        const Function_Arg_Descriptor& arg  = _func_type.arg_at(i);

        const char* name;
        // TODO: this could be done in the Node_View instead...
        if (m_node->m_type == Node_Type_OPERATOR )
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

        Node_Property* property  = m_node->add_prop(arg.type, name );

        if ( arg.pass_by_ref )
            property->set_flags(Node_Property_Flag_IS_REF);

        m_argument_slot[i]  = m_node->add_slot(property, Node_Slot_Flag_INPUT, 1);
        m_argument_props[i] = property;
    }
}

Node::Variable_State::Variable_State(Node* node)
: m_owner_node(node)
{
}

void Node::Variable_State::init(const tools::Type_Descriptor* _type, const char* _identifier)
{
    VERIFY( m_owner_node && m_owner_node->value() != nullptr, "Node does not seems to be initialized");

    auto value = m_owner_node->value();

    // Init identifier property
    value->set_type(_type);
    value->set_token({Token_Type::identifier});
    value->token().word_replace(_identifier); // might come from std::string::c_str()

    // Init Node_Slots
    m_owner_node->add_slot(value, Node_Slot_Flag_INPUT, 1); // to connect an initialization expression
    m_owner_node->add_slot(value, Node_Slot_Flag_FLOW_OUT, 1);
    m_owner_node->add_slot(value, Node_Slot_Flag_FLOW_IN);

    m_as_declaration_slot = m_owner_node->add_slot(value, Node_Slot_Flag_OUTPUT, 1); // as declaration
    m_as_reference_slot   = m_owner_node->add_slot(value, Node_Slot_Flag_OUTPUT); // as reference
}


Node::Variable_Ref_State::Variable_Ref_State(Node* node)
: m_owner_node(node)
{
}

Node::Variable_Ref_State::~Variable_Ref_State()
{
    clear_variable();
}

void Node::Variable_Ref_State::init()
{
    VERIFY( m_owner_node && m_owner_node->value() != nullptr, "Node does not seems to be initialized");

    auto value = m_owner_node->value();

    // Init identifier property
    value->set_type(tools::type::any());
    value->set_token({Token_Type::identifier});

    // Init Node_Slots
    m_owner_node->add_slot(value, Node_Slot_Flag_FLOW_OUT, 1);
    m_owner_node->add_slot(value, Node_Slot_Flag_FLOW_IN);
    m_owner_node->add_slot(value, Node_Slot_Flag_INPUT   , 1);
    m_owner_node->add_slot(value, Node_Slot_Flag_OUTPUT  , 1); // ref can be connected once
}

void Node::Variable_Ref_State::set_variable(Node* variable_node)
{
    VERIFY( m_variable_node == nullptr, "Can't call twice");

    m_variable_node = variable_node;

    auto& variable_data = m_variable_node->variable_data();
    m_owner_node->value()->set_type( variable_data.get_type() );
    m_owner_node->value()->token().word_replace( variable_data.get_identifier().c_str() );

    // bind signals
    m_variable_node->signal_name_change.connect<&Variable_Ref_State::handle_name_change>(this);
    m_variable_node->signal_shutdown.connect<&Variable_Ref_State::clear_variable>(this);
}

void Node::Variable_Ref_State::clear_variable()
{
    if ( m_variable_node == nullptr )
        return;

    // unbind signals
    m_variable_node->signal_name_change.disconnect();
    m_variable_node->signal_shutdown.disconnect();

    m_variable_node = nullptr;
}

const Token& Node::Variable_Ref_State::get_identifier_token() const
{
    return m_owner_node->value()->token(); // when parsed, this token may be a bit different from m_variable's (trailing ignored characters)
}

void Node::Variable_Ref_State::handle_name_change(const std::string& name)
{
    m_owner_node->value()->token().word_replace( name.c_str() );
}

Node::Literal_State::Literal_State(Node* node)
: node(node)
{
}

void Node::Literal_State::init(const Type_Descriptor* _type)
{
    VERIFY( node && node->value() != nullptr, "Node does not seems to be initialized");

    auto value = node->value();

    value->set_type(_type);

    node->add_slot(value, Node_Slot_Flag_FLOW_OUT , 1);
    node->add_slot(value, Node_Slot_Flag_FLOW_IN);
    node->add_slot(value, Node_Slot_Flag_OUTPUT   , 1);
}


void Node::switch_behavior_create_branches(size_t branch_count)
{
    VERIFY( 1 < branch_count && branch_count <= Switch_Behavior_State::BRANCH_MAX, "branch_count is out of range");
    VERIFY( has_switch_behavior(), "Node does not have a switch behavior" );


    m_switch_behavior_data.m_branch_count = branch_count;

    add_slot(value(), Node_Slot_Flag_FLOW_IN);      // accepts N inputs
    add_slot(value(), Node_Slot_Flag_FLOW_OUT , 1); // accepts 0 or 1 output

    // add 1 slot per branch
    for(size_t branch = 0; branch < branch_count; ++branch )
    {
        m_switch_behavior_data.m_branch_slot[branch] = add_slot(value(), Node_Slot_Flag_FLOW_ENTER, 1, branch);
    }

    // add 1 condition per branch except for the default branch
    for(size_t branch = 1; branch < branch_count; ++branch )
    {
        auto condition_property = add_prop<tools::any>(CONDITION_PROPERTY);
        m_switch_behavior_data.m_condition_in[branch-1]  = add_slot(condition_property, Node_Slot_Flag_INPUT, 1, branch);
    }
}

Node* ndbl::node_create_variable(const Type_Descriptor* _type, const std::string& _name)
{
    // create
    auto node = new Node(Node_Type_VARIABLE);
    node->init("Var.");
    node->variable_data().init(_type, _name.c_str());
    return node;
}

Node* ndbl::node_create_variable_ref()
{
    auto node = new Node(Node_Type_VARIABLE_REF);
    node->init("Ref.");
    node->variable_ref_data().init();
    return node;
}

Node* ndbl::node_create_function(const Function_Descriptor& _func_type, Node_Type _node_type)
{
    ASSERT(_node_type == Node_Type_OPERATOR || _node_type == Node_Type_FUNCTION );

    auto* node = new Node(_node_type);

    node->init(_func_type.get_identifier());
    node->invokable_data().init(_func_type);
    
    return node;
}

Node* ndbl::node_create_cond_struct()
{
    auto* node = new Node(Node_Type_IF_ELSE);

    node->init("If");
    node->init_internal_scope();
    node->switch_behavior_create_branches(2);
    node->switch_behavior_data().m_branch_prefix = {Token_Type::keyword_if};

    return node;
}

Node* ndbl::node_create_for_loop()
{
    auto* node = new Node(Node_Type_FOR_LOOP);

    node->init("For");

    node->switch_behavior_data().m_branch_prefix = {Token_Type::keyword_for};

    // add initialization property and slot
    auto* init_prop = node->add_prop<any>(INITIALIZATION_PROPERTY);
    node->switch_behavior_data().m_initialization_slot = node->add_slot(init_prop, Node_Slot_Flag_INPUT, 1);

    // add conditional-related properties and slots
    node->init_internal_scope();
    node->switch_behavior_create_branches(2);

    // add iteration property and slot
    auto iter_prop = node->add_prop<any>(ITERATION_PROPERTY);
    node->switch_behavior_data().m_iteration_slot = node->add_slot(iter_prop, Node_Slot_Flag_INPUT, 1);

    return node;
}

Node* ndbl::node_create_while_loop()
{
    auto* node = new Node(Node_Type_WHILE_LOOP);    
    node->init("While");
    node->init_internal_scope();
    node->switch_behavior_create_branches(2);
    node->switch_behavior_data().m_branch_prefix = {Token_Type::keyword_while};
    return node;
}

Node* ndbl::node_create_scope()
{
    auto* node = new Node(Node_Type_SCOPE);
    node->init("Scope");
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_IN);
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_OUT, 1);
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_OUT | Node_Slot_Flag_IS_INTERNAL, 1);
    node->init_internal_scope();
    return node;
}

Node* ndbl::node_create_root_scope()
{
    auto* node = new Node(Node_Type_ROOT);
    node->init(ICON_FA_ARROW_ALT_CIRCLE_DOWN " BEGIN");
    // add_slot(node->value(), Node_Slot_Flag_FLOW_IN, Node_Slot::MAX_CAPACITY); nothing can be before...
    // add_slot(node->value(), Node_Slot_Flag_FLOW_OUT, 1); nothing after either...
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_OUT | Node_Slot_Flag_IS_INTERNAL, 1); // ...but something inside!
    node->init_internal_scope();
    return node;
}

Node* ndbl::node_create_node()
{
    auto* node = new Node;
    node->init("");
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_OUT, 1);
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_IN);
    return node;
}

Node* ndbl::node_create_literal(const Type_Descriptor *_type)
{
    auto* node = new Node(Node_Type_LITERAL);
    node->init("Lit.");
    node->literal_data().init(_type);
    return node;
}

Node* ndbl::node_create_empty_instruction()
{
    auto* node = new Node(Node_Type_EMPTY_INSTRUCTION);
    node->init(";");

    // Token will be/or not overriden as a Token_t::end_of_instruction by the parser
    node->value()->set_token({Token_Type::ignore});

    node->add_slot(node->value(), Node_Slot_Flag_FLOW_OUT, 1);
    node->add_slot(node->value(), Node_Slot_Flag_FLOW_IN);
    node->add_slot(node->value(), Node_Slot_Flag_OUTPUT  , 1);

    return node;
}

std::vector<Node*> ndbl::node_get_adjacent_nodes(const Node* _node, Node_Slot_Flags _flags)
{
    std::vector<Node*> result;
    for ( Node_Slot* slot : _node->filter_slots(_flags ) )
    {
        for( const Node_Slot* adjacent : slot->adjacent() )
        {
            result.emplace_back(adjacent->node );
        }
    }
    return result;
}

Node* ndbl::node_adjacent_node_at(const Node* _node, Node_Slot_Flags _flags, u8_t _pos)
{
    if ( Node_Slot* adjacent_slot = _node->find_adjacent_at(_flags, _pos ) )
    {
        return adjacent_slot->node;
    }
    return {};
}

bool ndbl::node_is_instruction(const Node* node)
{
    if ( node_is_connected_to_codeflow(node) )
        return true;
    if (node->type() == Node_Type_VARIABLE )
        return true;
    return false;
}

bool ndbl::node_is_connected_to_codeflow(const Node *node)
{
    if (node->flow_inputs().size() )
        return true;
    if (node->flow_outputs().size() )
        return true;
    return false;
}

bool ndbl::node_can_be_instruction(const Node* node)
{
    // TODO: handle case where a variable has inputs/outputs but not connected to the code flow
    return node->slot_count(Node_Slot_Flag_TYPE_FLOW) > 0 && node->inputs().empty() && node->outputs().empty();
}

bool ndbl::node_is_unary_operator(const Node* node)
{
    if (node->type() == Node_Type_OPERATOR )
        if (node->invokable_data().get_func_type()->arg_count() == 1 )
            return true;
    return false;
}

bool ndbl::node_is_binary_operator(const Node* node)
{
    if (node->type() == Node_Type_OPERATOR )
        if (node->invokable_data().get_func_type()->arg_count() == 2 )
            return true;
    return false;
}

bool ndbl::node_is_conditional(const Node* node)
{
    switch ( node->type() )
    {
        case Node_Type_FOR_LOOP:
        case Node_Type_WHILE_LOOP:
        case Node_Type_IF_ELSE:
            return true;
        default:
            return false;
    };
}

bool ndbl::node_is_output_node_in_expression(const Node* input_node, const Node* output_node)
{
#ifdef NDBL_DEBUG
    ASSERT(input_node);
    ASSERT(output_node);
    const bool is_an_output = std::find(input_node->outputs().begin(), input_node->outputs().end(), output_node) != input_node->outputs().end();
    ASSERT(is_an_output);
#endif

    if ( node_is_instruction(input_node ) )
    {
        if (input_node->type() == Node_Type_VARIABLE )
        {
            const Node_Slot* declaration_out = input_node->variable_data().decl_out();
            return declaration_out->first_adjacent_node() == output_node;
        }
        return false;
    }
    return input_node->outputs().front() == output_node;
}

bool ndbl::node_is_initialized(const Node* node)
{
    return node != nullptr && node->is_initialized();
}
