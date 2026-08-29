#include "Node.h"

#include <algorithm> // for std::find
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "bdc/String.hpp"
#include "core/Asserts.h"
#include "core/Constants.h"
#include "Scope.h"
#include "Graph.h"
#include "core/Flags.h"
#include "core/Node_Property.h"
#include "core/Node_Slot.h"
#include "core/reflection/Type_Descriptor.h"

// private
namespace ndbl
{
    void                _node_init_tagged_union(Node* node);
    void                _node_deinit_tagged_union(Node* node);
    void                _node_update_adjacent_nodes_cache(const Node*, Node_Slot::Flags);
    std::vector<Node*>  _node_collect_adjacent_nodes(const Node*, Node_Slot::Flags);
}

std::vector<ndbl::Node*> ndbl::Node::inputs() const
{
    return node_get_adjacent_nodes(this, Node_Slot::Flag_INPUT);
}

std::vector<ndbl::Node*> ndbl::Node::outputs() const
{ 
    return node_get_adjacent_nodes(this, Node_Slot::Flag_OUTPUT);
}

std::vector<ndbl::Node*> ndbl::Node::flow_inputs() const
{
    return node_get_adjacent_nodes(this, Node_Slot::Flag_FLOW_IN);
}

std::vector<ndbl::Node*> ndbl::Node::flow_outputs() const
{ 
    return node_get_adjacent_nodes(this, Node_Slot::Flag_FLOW_OUT);
}

std::vector<ndbl::Node*> ndbl::node_get_adjacent_nodes(const Node* node, Node_Slot::Flags flags)
{
    return _node_collect_adjacent_nodes(node, flags );

    // _node_update_adjacent_nodes_cache(node, flags);
    // return node->adjacent_nodes_cache[flags];
}

ndbl::Node::Node(const Node& other)
{
   *this = other;
}

ndbl::Node& ndbl::Node::operator=(const Node& other)
{
    if( &other == this )
    {
        return *this;
    }
    
    #warning Not implemented yet!
    memcpy((void*)this, (void*)&other, sizeof(Node) );

    return *this;
}

ndbl::Node::~Node()
{
    #warning Not implemented yet!
}

void ndbl::node_init(Node* node, Node_Type type, const bdc::String& label)
{
    ASSERT( node != nullptr );
    ASSERT( type != Node_Type_NULL );
    VERIFY( !HAS_FLAGS(node->flags, Node_Flag_IS_INITIALIZED), "You cannot initialize twice");

    bdc::hashmap_init(node->props_by_name);
    node->flags = Node_Flag_IS_DIRTY;
    node->type  = type;
    node->value = node_add_prop<tools::any>(node, DEFAULT_PROPERTY, Node_Property::Flag_IS_NODE_VALUE );
    
    node_set_name(node, label);
    _node_init_tagged_union(node);

    node->flags |= Node_Flag_IS_INITIALIZED;
}

void ndbl::_node_init_tagged_union(Node* node)
{
    switch (node->type)
    {
        case Node_Type_IF_ELSE:    [[fallthrough]];     
        case Node_Type_WHILE_LOOP: [[fallthrough]];
        case Node_Type_FOR_LOOP:
        {
            new (&node->switch_data) ndbl::Node::Switch_Behavior_State();
            break;
        }

        case Node_Type_LITERAL:
        {
            new (&node->literal_data) ndbl::Node::Literal_State();            
            break;
        }

        case Node_Type_OPERATOR: [[fallthrough]];
        case Node_Type_FUNCTION:
        {
            new (&node->invokable_data) ndbl::Node::Invokable_State();
            break;
        }

        case Node_Type_VARIABLE_REF:
        {
            new (&node->variableref_data) ndbl::Node::Variable_Ref_State();
            break;
        }

        case Node_Type_VARIABLE:
        {
            new (&node->variableref_data) ndbl::Node::Variable_State();
            break;
        }

        case Node_Type_RETURN:            [[fallthrough]];
        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             [[fallthrough]];
        case Node_Type_EMPTY_INSTRUCTION: [[fallthrough]];
        case Node_Type_NULL:
            // Those types do not have a dedicated data struct in the union
            break;

        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            TOOLS_UNREACHABLE("Unhandled Node_Type (value: %i)\n", node->type);
    }
}

void ndbl::node_deinit(Node* node)
{
    ASSERT(node != nullptr);

    string_release(node->name);

    while( !node->props.empty() )
    {
        auto result = bdc::hashmap_remove( node->props_by_name, node->props.back()->name );
        ASSERT( result.ok );
        property_release(node->props.back());
        node->props.pop_back();
    }
    bdc::hashmap_release(node->props_by_name);

    while( !node->slots.empty() )
    {
        delete node->slots.back();
        node->slots.pop_back();
    }

    _node_deinit_tagged_union(node);

    if( node->view )
    {
        nodeview_deinit(node->view);
        bdc::memory_free(node->view);
    }

    if( node->internal_scope )
    {
        scope_deinit(node->internal_scope);
        bdc::memory_free(node->internal_scope);
    }

    node->signal_deinit.emit();

    node->~Node();
}

void ndbl::_node_deinit_tagged_union(Node* node)
{
switch (node->type)
    {
        case Node_Type_IF_ELSE:    [[fallthrough]];
        case Node_Type_WHILE_LOOP: [[fallthrough]];
        case Node_Type_FOR_LOOP:
        {
            node->switch_data.~Switch_Behavior_State();
            break;
        }

        case Node_Type_LITERAL:
        {
            node->literal_data.~Literal_State();
            break;
        }

        case Node_Type_OPERATOR: [[fallthrough]];
        case Node_Type_FUNCTION:
        {
            node->invokable_data.~Invokable_State();
            break;
        }

        case Node_Type_VARIABLE_REF:
        {
            node_variable_ref_clear_variable(node);
            node->variableref_data.~Variable_Ref_State();
            break;
        }

        case Node_Type_VARIABLE:
        {
            node->variable_data.~Variable_State();
            break;
        }

        case Node_Type_RETURN:            [[fallthrough]];
        case Node_Type_ROOT:              [[fallthrough]];
        case Node_Type_SCOPE:             [[fallthrough]];
        case Node_Type_EMPTY_INSTRUCTION: [[fallthrough]];
        case Node_Type_NULL:
            // Those types do not have a dedicated data struct in the union
            break;

        default:
            TOOLS_UNREACHABLE("Unhandled Node_Type (value: %i)\n", node->type);
    }
}

const ndbl::Type_Descriptor* ndbl::node_get_connected_function_type(const Node* node, const bdc::String& property_name)
{
    const Node_Slot* slot = node_find_slot_by_property_name(node, property_name, Node_Slot::Flag_INPUT );
    VERIFY(slot!= nullptr, "Unable to find input slot for this property name");
    const Node_Slot* adjacent_slot = slot->first_adjacent();

    if ( adjacent_slot )
        if ( node_is_invokable(adjacent_slot->node) )
            return &adjacent_slot->node->invokable_data.type;

    return nullptr;
}

const ndbl::Node_Slot* ndbl::node_find_slot_by_property_name(const Node* node, const bdc::String& property_name, Node_Slot::Flags desired_way)
{
    const Node_Property* property = node_find_prop_by_name(node, property_name);
    if( property )
    {
        return node_find_slot_by_property( node, property, desired_way );
    }
    return nullptr;
}

const ndbl::Node_Slot* ndbl::node_find_slot_at(const Node* node, Node_Slot::Flags flags, size_t position)
{
    for( const Node_Slot* slot : node->slots )
    {
        if( HAS_FLAGS(slot->flags, flags) && slot->position == position && slot->property == node->value )
        {
            return slot;
        }
    }
    return nullptr;
}

ndbl::Node_Slot* ndbl::node_find_slot_by_property_type(const Node* node, Node_Slot::Flags flags, const tools::Type_Descriptor* type)
{
    for(Node_Slot* slot : node_filter_slots(node, flags) )
    {
        if( tools::type_is_implicitly_convertible(slot->property->type, type ) )
        {
            return slot;
        }
    }
    return nullptr;
}

void ndbl::Node::handle_slot_change(Node_Slot::Event event, Node_Slot* slot)
{
    this->adjacent_nodes_cache.clear();
}

void ndbl::node_set_name(Node* node, const bdc::String& new_name)
{
    node->name = string_copy(new_name);
    node->signal_name_change.emit(node->name);
}

ndbl::Node_Slot* ndbl::node_add_slot(Node* node, Node_Property* property, Node_Slot::Flags flags, size_t capacity, size_t position)
{
    ASSERT( property != nullptr );
    ASSERT( property->node == node );
    if ( (flags & Node_Slot::Flag_FLOW_OUT) == Node_Slot::Flag_FLOW_OUT)
    {
        VERIFY( capacity == 1, "Node_Slot::Flag_FLOW_OUT can only have a capacity of 1" );
    }

    Node_Slot* slot = bdc::memory_new<Node_Slot>();
    node_slot_init(slot, flags, capacity, position);
    slot->node      = node;
    slot->property  = property;

    node->slots.push_back( slot );
    bdc::array_append( property->slots, slot );

    // listen to events to clear cache
    slot->signal_change.connect<&ndbl::Node::handle_slot_change>(node);

    return slot;
}

std::vector<ndbl::Node_Slot*> ndbl::node_filter_adjacent_slots(const Node* node, Node_Slot::Flags flags )
{
    std::vector<Node_Slot*> result;

    for(Node_Slot* slot : node_filter_slots(node, flags))
        for( Node_Slot* each : slot->adjacent )
            result.push_back( each );

    return result;
}

bool ndbl::node_has_input_connected(const Node* node, const Node_Property* property )
{
    const Node_Slot* slot = node_find_slot_by_property(node, property, Node_Slot::Flag_INPUT );
    return slot && slot->adjacent.size > 0;
}

const ndbl::Node_Slot* ndbl::node_find_slot_by_property(const Node* node, const Node_Property* prop, Node_Slot::Flags flags)
{
    for(u32_t i = 0; i < prop->slots.size; ++i)
    {
        if( HAS_FLAGS(prop->slots[i]->flags, flags) )
        {
            return prop->slots[i];
        }
    }

    return nullptr;
}

ndbl::Node_Slot* ndbl::node_find_adjacent_at(const Node* node, Node_Slot::Flags _flags, size_t _index )
{
    size_t cursor_pos{0};
    for (Node_Slot* slot : node->slots)
    {
        // Skip any slot not compatible with given flags
        if( !HAS_FLAGS(slot->flags, _flags ) )
        {
            continue;
        }

        // if the position is in the range of this slot, we return the item
        size_t local_pos = (size_t)_index - cursor_pos;
        if ( local_pos < slot->adjacent.size )
        {
            return node_slot_adjacent_at(slot, local_pos);
        }
        // increase counter
        cursor_pos += slot->adjacent.size;
    }
    return nullptr;
}

std::vector<ndbl::Node_Slot*> ndbl::node_filter_slots(const Node* node, Node_Slot::Flags flags)
{
    const auto if_has_flags = [flags](const Node_Slot* _slot)
    {
        ASSERT_DEBUG_ONLY(_slot != nullptr);
        return HAS_FLAGS(_slot->flags, flags);
    };
    return node_filter_slots(node, if_has_flags);
}

std::vector<ndbl::Node_Slot*> ndbl::node_filter_slots(const Node* node, const std::function<bool(const Node_Slot*)>& predicate)
{
    std::vector<Node_Slot*> result;
    std::copy_if( node->slots.begin(), node->slots.end(), std::back_inserter(result), predicate);
    return result;
}

ndbl::Node_Slot* ndbl::Node::value_out()
{
    return const_cast<Node_Slot*>( node_find_slot_by_property(this, value, Node_Slot::Flag_OUTPUT ) );
}

const ndbl::Node_Slot* ndbl::Node::value_out() const
{
    return node_find_slot_by_property(this, value, Node_Slot::Flag_OUTPUT );
}

ndbl::Node_Slot* ndbl::Node::value_in()
{
    return const_cast<Node_Slot*>( node_find_slot_by_property(this, value, Node_Slot::Flag_INPUT ) );
}

const ndbl::Node_Slot* ndbl::Node::value_in() const
{
    return node_find_slot_by_property(this, value, Node_Slot::Flag_INPUT );
}

ndbl::Node_Slot* ndbl::Node::flow_enter()
{
    auto* const_this = const_cast<const Node*>(this);
    return const_cast<Node_Slot*>( const_this->flow_enter());
}

const ndbl::Node_Slot* ndbl::Node::flow_enter() const
{
    for(u32_t i = 0; i < value->slots.size; ++i)
    {
        if( HAS_FLAGS(value->slots[i]->flags, Node_Slot::Flag_FLOW_ENTER) )
        {
            return value->slots[i];
        }
    }
    return nullptr;
}

ndbl::Node_Slot* ndbl::Node::flow_out()
{
    auto* const_this = const_cast<const Node*>(this);
    return const_cast<Node_Slot*>( const_this->flow_out());
}

const ndbl::Node_Slot* ndbl::Node::flow_out() const
{
    if (u32_t i = 0; i < value->slots.size )
    {
        const ndbl::Node_Slot* slot = value->slots[i];
        if( HAS_FLAGS( slot->flags, Node_Slot::Flag_FLOW_OUT) )
            if (!HAS_FLAGS(slot->flags, Node_Slot::Flag_IS_INTERNAL) ) // branches (internal) are specific flow_out, we don't want to grab them here
                return slot;
    }
    return nullptr;
}

ndbl::Node_Slot* ndbl::Node::flow_in()
{
    return const_cast<Node_Slot*>( node_find_slot_by_property(this, value, Node_Slot::Flag_FLOW_IN ) );
}

const ndbl::Node_Slot* ndbl::Node::flow_in() const
{
    return node_find_slot_by_property(this, value, Node_Slot::Flag_FLOW_IN );
}

bool ndbl::node_update(Node* node)
{
    node->flags = Node_Flag_IS_DIRTY;
    return true;
}

void ndbl::_node_update_adjacent_nodes_cache(const Node* node, Node_Slot::Flags flags)
{
     if ( node->adjacent_nodes_cache.find(flags) == node->adjacent_nodes_cache.end() )
    {
        node->adjacent_nodes_cache.insert_or_assign(flags, _node_collect_adjacent_nodes(node, flags ) );
    }
}

void ndbl::node_init_internal_scope(Node* node)
{
    VERIFY( node->internal_scope == nullptr, "Can't call init_internal_scope() more than once");
    VERIFY( node->scope == nullptr, "Must be initialized prior to reset_parent()");

    auto* scope = bdc::memory_new<Scope>();
    scope_init(scope);
    scope->name = "Internal Scope";
    scope->node = node;

    node->internal_scope = scope;
}

bool ndbl::node_has_flow_adjacent(const Node* node)
{
    return !node->flow_inputs().empty() || !node->flow_outputs().empty();
}

bool ndbl::node_has_switch_behavior(const Node* node)
{
    switch (node->type)
    {
    case Node_Type_FOR_LOOP:
    case Node_Type_IF_ELSE:
    case Node_Type_WHILE_LOOP:
        return true;
    
    default:
        return false;
    }
}

bool ndbl::node_is_expression(const Node* node)
{
    return !node->inputs().empty();
}

void ndbl::node_reset_scope(Node* node, Scope* scope)
{
#ifdef TOOLS_DEBUG
    if ( scope == nullptr )
        VERIFY( node->flags & Node_Flag_WAS_IN_A_SCOPE_ONCE, "This node never been in a scope, why would you reset it to nullptr? (that's the default value)")
#endif
    node->flags |= Node_Flag_WAS_IN_A_SCOPE_ONCE;
    node->scope = scope;

    if ( node->internal_scope != nullptr )
    {
        scope_reset_parent( node->internal_scope, scope );
    }
}

bool ndbl::node_has_prop(const Node* node, const bdc::String& name)
{
    return bdc::hashmap_find(node->props_by_name, name);
}

ndbl::Node_Property* ndbl::node_add_prop(Node* node, const tools::Type_Descriptor* type, const bdc::String name, Node_Property::Flags flags )
{
    // guards
    VERIFY(!node_has_prop(node, name), "Property name already used");

    // create
    auto* new_property = bdc::memory_new<Node_Property>(); // TODO: use a static-sized array with a given limit (ex: 10 props)
    property_init(new_property, node, type, flags, name);

    // register / index
    node->props.push_back(new_property);
    auto result = bdc::hashmap_add(node->props_by_name, new_property->name, new_property);
    ASSERT(result.ok);

    return new_property;
}

const ndbl::Node_Property* ndbl::node_find_first_prop(const Node* node, Node_Property::Flags _flags, const tools::Type_Descriptor *_type)
{
    auto filter = [_flags, _type](const Node_Property* property) -> bool
    {
        return tools::type_is_implicitly_convertible(property->type, _type)
               && ( HAS_FLAGS(property->flags, _flags ) );
    };

    HASHMAP_WALK( entry, node->props_by_name )        
        if( filter(entry.value) )
        {
            return entry.value;
        }
    HASHMAP_WALK_END

    return nullptr;
}

const ndbl::Node_Property* ndbl::node_find_prop_by_name(const Node* node, const bdc::String& name)
{
    HASHMAP_WALK( entry, node->props_by_name )        
        if( entry.key == name )
        {
            return entry.value;
        }
    HASHMAP_WALK_END

    ASSERT(false);
    return nullptr;
}

void ndbl::node_init_as_invokable(Node* node, const tools::Type_Descriptor* function_desc, Node_Type node_type )
{
    ASSERT(node != nullptr);
    ASSERT(node_type == Node_Type_OPERATOR || node_type == Node_Type_FUNCTION );

    node_init(node, node_type, function_desc->name);
    node->invokable_data.type = *function_desc;
    node->invokable_data.identifier_token = {
            Token_Type_identifier,
            function_desc->name
    };
    array_resize( node->invokable_data.argument_slots, function_desc->function.args.size);
    array_resize( node->invokable_data.argument_props, function_desc->function.args.size);

    switch ( node->type )
    {
        case Node_Type_OPERATOR:
            node_set_name(node, function_desc->name);
            break;
        case Node_Type_FUNCTION:
        {
            const bdc::String& id   = function_desc->name;
            bdc::String label       = id; // We add dynamically the brackets (see Node_View)
            bdc::String short_label = bdc::string_printf("%.2s..", id.c_str());
            node_set_name(node, label);
            break;
        }
        default:
            VERIFY(false, "Type not allowed");
    }

    // Create a result/value
    property_set_type(node->value, function_desc->function.return_type );

    node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT );
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN );

    // Create arguments
    if (node->type == Node_Type_OPERATOR )
    {
        VERIFY(function_desc->function.args.size >= 1, "An operator must have one argument minimum");
        VERIFY(function_desc->function.args.size <= 2, "An operator cannot have more than 2 arguments");
    }

    for (size_t i = 0; i < function_desc->function.args.size; i++ )
    {
        const tools::Function_Arg_Descriptor& arg  = function_desc->function.args[i];

        bdc::String name;
        // TODO: this could be done in the Node_View instead...
        if (node->type == Node_Type_OPERATOR )
        {
            if ( i == 0 )
                name = LEFT_VALUE_PROPERTY ;
            else if ( i == 1 )
                name = RIGHT_VALUE_PROPERTY;
        }
        else
        {
            name = arg.name;
        }

        Node_Property* property  = node_add_prop(node, arg.type, name );

        SET_FLAGS_VALUE(property->flags, Node_Property::Flag_IS_REF, arg.pass_by_ref);

        node->invokable_data.argument_slots[i]  = node_add_slot(node, property, Node_Slot::Flag_INPUT, 1);
        node->invokable_data.argument_props[i] = property;
    }
}

void ndbl::node_init_as_variable(Node* node, const tools::Type_Descriptor* _type, const bdc::String _identifier)
{
    node_init(node, Node_Type_VARIABLE, "Var.");

    // Init identifier property
    property_set_type(node->value, _type);
    node->value->token = Token{Token_Type_identifier};
    node->value->token.replace_word(_identifier); // might come from bdc::String::c_str()

    // Init Node_Slots
    node_add_slot(node, node->value, Node_Slot::Flag_INPUT, 1); // to connect an initialization expression
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);

    node->variable_data.type_token        = {Token_Type_keyword_unknown }; // [int] var  =
    node->variable_data.operator_token    = {Token_Type_operator };       //  int  var [=]
    node->variable_data.flags             = VariableFlag_NONE;
    node->variable_data.decl_out = node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT, 1); // as declaration
    node->variable_data.ref_out  = node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT); // as reference
}

void ndbl::node_init_as_variable_ref(Node* node)
{
    node_init(node, Node_Type_VARIABLE_REF, "Ref.");

    // Init identifier property
    property_set_type(node->value, tools::type_any());
    node->value->token = Token{Token_Type_identifier};

    // Init Node_Slots
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
    node_add_slot(node, node->value, Node_Slot::Flag_INPUT   , 1);
    node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT  , 1); // ref can be connected once
}

void ndbl::node_variable_ref_set_variable(Node* node, Node* variable_node)
{
    ASSERT_DEBUG_ONLY(variable_node != nullptr);
    VERIFY( node->variableref_data.variable_node == nullptr, "Can't call twice");

    node->variableref_data.variable_node = variable_node;

    property_set_type(node->value, node_variable_type(variable_node) );
    node->value->token.replace_word( node_get_identifier(variable_node).c_str() );

    // bind signals
    node->variableref_data.variable_node->signal_name_change.connect< &node_variable_ref_handle_name_change>(node);
    node->variableref_data.variable_node->signal_deinit.connect<&node_variable_ref_clear_variable>(node);
}

void ndbl::node_variable_ref_clear_variable(Node* node)
{
    Node* variable_node = node->variableref_data.variable_node; 
    if ( variable_node == nullptr )
        return;

    // unbind signals
    variable_node->signal_name_change.disconnect();
    variable_node->signal_deinit.disconnect();
    variable_node = nullptr;
}

void ndbl::node_variable_ref_handle_name_change(Node* node, const bdc::String& name)
{
    node->value->token.replace_word( name.c_str() );
}

void ndbl::node_init_as_literal(Node* node, const tools::Type_Descriptor* type_descriptor)
{
    node_init(node, Node_Type_LITERAL, "Lit.");
    
    property_set_type(node->value, type_descriptor);

    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
    node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT   , 1);
    
    node->literal_data.token = {Token_Type_literal_any};
    node->literal_data.type  = nullptr;
}


void ndbl::node_init_branches(Node* node, size_t branch_count)
{
    VERIFY( 1 < branch_count && branch_count <= ndbl::Node::Switch_Behavior_State::BRANCH_MAX, "branch_count is out of range");
    VERIFY( node_has_switch_behavior(node), "Node does not have a switch behavior" );


    node->switch_data.branch_count = branch_count;

    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);      // accepts N inputs
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1); // accepts 0 or 1 output

    // add 1 slot per branch
    for(size_t branch = 0; branch < branch_count; ++branch )
    {
        node->switch_data.branch_slots[branch] = node_add_slot(node, node->value, Node_Slot::Flag_FLOW_ENTER, 1, branch);
    }

    // add 1 condition per branch except for the default branch
    for(size_t branch = 1; branch < branch_count; ++branch )
    {
        auto condition_property = node_add_prop<tools::any>(node, CONDITION_PROPERTY);
        node->switch_data.condition_in_slots[branch-1]  = node_add_slot(node, condition_property, Node_Slot::Flag_INPUT, 1, branch);
    }
}

void ndbl::node_init_as_cond_struct(Node* node)
{
    node_init(node, Node_Type_IF_ELSE, "If");
    node_init_internal_scope(node);
    node_init_branches(node, 2);
    node->switch_data.branch_prefix = Token_Type_keyword_if;
}

void ndbl::node_init_as_for_loop(Node* node)
{
    node_init(node, Node_Type_FOR_LOOP, "For");

    node->switch_data.branch_prefix = Token_Type_keyword_for;

    // add initialization property and slot
    Node_Property* init_prop = node_add_prop<tools::any>(node, INITIALIZATION_PROPERTY);
    node->switch_data.initialization_slot = node_add_slot(node, init_prop, Node_Slot::Flag_INPUT, 1);

    // add conditional-related properties and slots
    node_init_internal_scope(node);
    node_init_branches(node, 2);

    // add iteration property and slot
    Node_Property* iter_prop = node_add_prop<tools::any>(node, ITERATION_PROPERTY);
    node->switch_data.iteration_slot = node_add_slot(node, iter_prop, Node_Slot::Flag_INPUT, 1);
}

void ndbl::node_init_as_while_loop(Node* node)
{  
    node_init(node, Node_Type_WHILE_LOOP, "While");
    node_init_internal_scope(node);
    node_init_branches(node, 2);
    node->switch_data.branch_prefix = {Token_Type_keyword_while};
}

void ndbl::node_init_as_return(Node* node, const tools::Type_Descriptor* type_descriptor)
{
    node_init(node, Node_Type_RETURN, "Return");

    if ( type_descriptor == nullptr)
    {
        type_descriptor = tools::type_get<void>();
    }
    
    property_set_type(node->value, type_descriptor);
    node_add_slot(node, node->value, Node_Slot::Flag_INPUT, 1);

    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1); // nothing prevents for writing something after a return, at runtime it would not be executed, but in Nodable we do not care about that, we focus the code.
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
    // node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT   , 1); // we CANNOT use return's value in an expression! Of course!
}

void ndbl::node_init_as_scope(Node* node)
{
    node_init(node, Node_Type_SCOPE, "Scope");
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT | Node_Slot::Flag_IS_INTERNAL, 1);
    node_init_internal_scope(node);
}

void ndbl::node_init_as_root_scope(Node* node)
{
    node_init(node, Node_Type_ROOT, ICON_FA_ARROW_ALT_CIRCLE_DOWN " BEGIN");
    // add_slot(node->value(), Node_Slot::Flag_FLOW_IN, Node_Slot::MAX_CAPACITY); nothing can be before...
    // add_slot(node->value(), Node_Slot::Flag_FLOW_OUT, 1); nothing after either...
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT | Node_Slot::Flag_IS_INTERNAL, 1); // ...but something inside!
    node_init_internal_scope(node);
}

void ndbl::node_init_as_empty_instruction(Node* node)
{
    node_init(node, Node_Type_EMPTY_INSTRUCTION, ";");

    // Token will be/or not overriden as a Token_t::end_of_instruction by the parser
    node->value->token = Token{Token_Type_NULL};

    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
    node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT  , 1);
}

std::vector<ndbl::Node*> ndbl::_node_collect_adjacent_nodes(const Node* node, Node_Slot::Flags flags)
{
    std::vector<Node*> result;
    for ( Node_Slot* slot : node_filter_slots(node, flags ) )
    {
        for( const Node_Slot* adjacent : slot->adjacent )
        {
            result.emplace_back(adjacent->node );
        }
    }
    return result;
}

ndbl::Node* ndbl::node_adjacent_node_at(const Node* node, Node_Slot::Flags flags, u8_t pos)
{
    if ( Node_Slot* adjacent_slot = node_find_adjacent_at(node, flags, pos ) )
    {
        return adjacent_slot->node;
    }
    return {};
}

bool ndbl::node_is_instruction(const Node* node)
{
    if ( node_is_connected_to_codeflow(node) )
        return true;
    if ( node->type == Node_Type_VARIABLE )
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

bool ndbl::node_could_be_instruction(const Node* node)
{
    // TODO: handle case where a variable has inputs/outputs but not connected to the code flow
    return node_slot_count(node, Node_Slot::Flag_TYPE_FLOW) > 0 && node->inputs().empty() && node->outputs().empty();
}

bool ndbl::node_is_unary_operator(const Node* node)
{
    if (node->type == Node_Type_OPERATOR )
        if (node->invokable_data.type.function.args.size == 1 )
            return true;
    return false;
}

bool ndbl::node_is_binary_operator(const Node* node)
{
    if (node->type == Node_Type_OPERATOR )
        if (node->invokable_data.type.function.args.size == 2 )
            return true;
    return false;
}

bool ndbl::node_is_conditional(const Node* node)
{
    switch ( node->type )
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
        if ( input_node->type == Node_Type_VARIABLE )
        {
            const Node_Slot* declaration_out = input_node->variable_data.decl_out;
            return declaration_out->first_adjacent_node() == output_node;
        }
        return false;
    }
    return input_node->outputs().front() == output_node;
}

bool ndbl::node_is_initialized(const Node* node)
{
    return node != nullptr && (node->flags & Node_Flag_IS_INITIALIZED);
}
