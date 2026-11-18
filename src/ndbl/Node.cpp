#include "Node.h"

#include <algorithm> // for std::find
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "bdc/String.hpp"
#include "Asserts.h"
#include "Constants.h"
#include "Flags.h"
#include "Node_Property.h"
#include "Node_Slot.h"
#include "reflection/Type_Descriptor.h"
#include "Scope.h"
#include "Graph.h"
#include "Parser.h"

namespace ndbl
{
using namespace bdc;

Node::Component_Kind to_component_type(Node_Kind);
           
Node::Component& Node::Component::operator=(const Node::Component& other)
{
    if( this == &other)
    {
        return *this;
    }

    if( kind != other.kind )
    {
        node_deinit_component(this, other.kind);
    }

    switch (other.kind)
    {
        case Component_Kind_NULL:           kind  = other.kind; break;
        case Component_Kind_BRANCHING:      branching       = other.branching;      break;
        case Component_Kind_INVOKABLE:      invokable       = other.invokable;      break;
        case Component_Kind_VARIABLE:       variable        = other.variable;       break;
        case Component_Kind_VARIABLE_REF:   variableref     = other.variableref;    break;
        case Component_Kind_LITERAL:        literal         = other.literal;        break;
    }

    return *this;
}

Node::Component::~Component()
{
    node_deinit_component(this, kind);
}
            
Array<Node*> Node::inputs() const
{
    return node_get_adjacent_nodes(this, Node_Slot::Flag_INPUT);
}

Array<Node*> Node::outputs() const
{ 
    return node_get_adjacent_nodes(this, Node_Slot::Flag_OUTPUT);
}

Array<Node*> Node::flow_inputs() const
{
    return node_get_adjacent_nodes(this, Node_Slot::Flag_FLOW_IN);
}

Array<Node*> Node::flow_outputs() const
{ 
    return node_get_adjacent_nodes(this, Node_Slot::Flag_FLOW_OUT);
}

Node_ID node_generate_new_id()
{
    static u32_t next_node_id = 1; // 0 is reserved for null

    Node_ID result = next_node_id;    
    next_node_id++;

    return result;
}

void node_init(Node* node, const Node_Init_Params& params)
{
    NDBL_ASSERT( node != nullptr );
    NDBL_ASSERT( !HAS_FLAGS(node->flags, Node_Flag_IS_INITIALIZED), "node was already initialized");

    node->kind   = params.kind;
    node->value  = node_add_prop<any>(node, DEFAULT_PROPERTY, Node_Property::Flag_IS_NODE_VALUE );
    node->id     = params.id == 0 ?  node_generate_new_id() : params.id;
    node->flags |= Node_Flag_IS_INITIALIZED;

    node_init_component(&node->component, to_component_type(node->kind) );

    switch ( params.kind )
    {
        case Node_Kind_EMPTY_INSTRUCTION:
        {
            node_set_name(node, "Instr.");

            // Token might be overriden as a Token_Type_end_of_instruction by the parser
            node->value->token = Token{Token_Type_NULL};

            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN    );
            node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT  , 1);
            break;
        }

        case Node_Kind_IF_ELSE:
        {
            node_set_name(node, "If");

            node_init_internal_scope(node);
            node_init_branches(node, 2);
            node->component.branching.branch_prefix = Token_Type_keyword_if;
            break;
        }

        case Node_Kind_FOR_LOOP:
        {
            node_set_name(node, "For");

            node->component.branching.branch_prefix = Token_Type_keyword_for;

            // add initialization property and slot
            Node_Property* init_prop = node_add_prop<any>(node, INITIALIZATION_PROPERTY);
            node->component.branching.initialization_slot = node_add_slot(node, init_prop, Node_Slot::Flag_INPUT, 1);

            // add conditional-related properties and slots
            node_init_internal_scope(node);
            node_init_branches(node, 2);

            // add iteration property and slot
            Node_Property* iter_prop = node_add_prop<any>(node, ITERATION_PROPERTY);
            node->component.branching.iteration_slot = node_add_slot(node, iter_prop, Node_Slot::Flag_INPUT, 1);

            break;
        }

        case Node_Kind_WHILE_LOOP:
        {  
            node_set_name(node, "While");

            node_init_internal_scope(node);
            node_init_branches(node, 2);
            node->component.branching.branch_prefix = {Token_Type_keyword_while};
            break;
        }

        case Node_Kind_SCOPE:
        {
            if( params.scope_is_root )
            {
                node_set_name(node, "BEGIN");
            }
            else
            {
                node_set_name(node, "Scope");
                node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
                node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
            }
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT | Node_Slot::Flag_IS_INTERNAL, 1);
            node_init_internal_scope(node);
            break;
        }

        case Node_Kind_VARIABLE_REF:
        {
            node_set_name(node, "Ref.");

            // Init identifier property
            property_set_type(node->value, type_any());
            node->value->token = Token{Token_Type_identifier};

            // Init Node_Slots
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
            node_add_slot(node, node->value, Node_Slot::Flag_INPUT   , 1);
            node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT  , 1); // ref can be connected once
            break;
        }

        // TODO: I don't need specific functions if they share the same parameters. A switch/case can do the job no?
        case Node_Kind_RETURN:
        {
            node_set_name(node, "Return");

            property_set_type(node->value, params.type_descriptor ? params.type_descriptor : type_get<void>() );
            node_add_slot(node, node->value, Node_Slot::Flag_INPUT, 1);

            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1); // nothing prevents for writing something after a return, at runtime it would not be executed, but in Nodable we do not care about that, we focus the code.
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
            // node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT   , 1); // we CANNOT use return's value in an expression! Of course!
            break;
        }     

        case Node_Kind_VARIABLE:
        {
            node_set_name(node, "Var.");

            auto* variable_type = params.type_descriptor;

            NDBL_ASSERT(
                variable_type == type_get<bool>() ||
                variable_type == type_get<double>() ||
                variable_type == type_get<int>() ||
                variable_type == type_get<bdc::String>(),
                "variable_type not allowed"
            );

            // Init identifier property
            property_set_type(node->value, variable_type);
            node->value->token = Token{Token_Type_identifier};
            token_replace_buffer_readonly(node->value->token, params.name); // might come from String::c_str()

            // Init Node_Slots
            node_add_slot(node, node->value, Node_Slot::Flag_INPUT, 1); // to connect an initialization expression
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);

            node->component.variable.type_token        = {Token_Type_keyword_unknown }; // [int] var  =
            node->component.variable.operator_token    = {Token_Type_operator };       //  int  var [=]
            node->component.variable.decl_out = node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT, 1); // as declaration
            node->component.variable.ref_out  = node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT); // as reference

            if( params.variable_with_assignment )
            {
                Token token = { Token_Type_keyword_operator, " = " };
                token.prefix_size = token.word_size = token.suffix_size = 1;
                
                node->component.variable.operator_token = token;
            }
            break;
        }
        
        case Node_Kind_LITERAL:
        {
            node_set_name(node, "Lit.");

            auto* literal_type = params.type_descriptor;

            NDBL_ASSERT(
                literal_type == type_get<bool>() ||
                literal_type == type_get<double>() ||
                literal_type == type_get<int>() ||
                literal_type == type_get<bdc::String>(),
                "literal_type not allowed"
            );

            node_set_name(node, "Lit.");
            
            property_set_type(node->value, literal_type);

            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1);
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);
            node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT   , 1);
            
            node->component.literal.token = {Token_Type_literal_any};
            node->component.literal.type  = nullptr;
            break;
        }
    
        case Node_Kind_OPERATOR:
        case Node_Kind_FUNCTION:
        {
            node_set_name(node, params.type_descriptor->name);
        
            node->component.invokable.type = *params.type_descriptor;
            node->component.invokable.identifier_token = {
                    Token_Type_identifier,
                    params.type_descriptor->name
            };
            array_resize( node->component.invokable.argument_slots, params.type_descriptor->function.args.size);
            array_resize( node->component.invokable.argument_props, params.type_descriptor->function.args.size);

            node_set_name(node, params.type_descriptor->name);

            // Create a result/value
            property_set_type(node->value, params.type_descriptor->function.return_type );

            node_add_slot(node, node->value, Node_Slot::Flag_OUTPUT );
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1);
            node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN );

            // Create arguments
            if (node->kind == Node_Kind_OPERATOR )
            {
                NDBL_ASSERT(params.type_descriptor->function.args.size >= 1, "An operator must have one argument minimum");
                NDBL_ASSERT(params.type_descriptor->function.args.size <= 2, "An operator cannot have more than 2 arguments");
            }

            for (size_t i = 0; i < params.type_descriptor->function.args.size; i++ )
            {
                String name;
                // TODO: this could be done in the Node_View instead...
                if (node->kind == Node_Kind_OPERATOR )
                {
                    if ( i == 0 )
                        name = LEFT_VALUE_PROPERTY ;
                    else if ( i == 1 )
                        name = RIGHT_VALUE_PROPERTY;
                }
                else
                {
                    name = bdc::string_tprintf( "arg_%i", i );;
                }

                const Function_Arg_Descriptor& arg  = params.type_descriptor->function.args[i];
                Node_Property* property = node_add_prop(node, arg.type, name );

                SET_FLAGS_VALUE(property->flags, Node_Property::Flag_IS_REF, arg.pass_by_ref);

                node->component.invokable.argument_slots[i] = node_add_slot(node, property, Node_Slot::Flag_INPUT, 1);
                node->component.invokable.argument_props[i] = property;
            }
            break;
        }

        case Node_Kind_NULL:
        {
            node_set_name(node, params.name);
            break;
        }
        
        default:
            NDBL_UNREACHABLE("Unexpected Node_Kind: %i\n", params.kind);
    }

    NDBL_ASSERT(node != nullptr);

    if ( params.created_by_the_user )
    {
        switch ( params.kind )
        {
            case Node_Kind_IF_ELSE:
            case Node_Kind_FOR_LOOP:
            case Node_Kind_WHILE_LOOP:
            case Node_Kind_SCOPE:
                node->suffix = token_end_of_line;
                break;
            case Node_Kind_VARIABLE:
            case Node_Kind_RETURN:
            case Node_Kind_EMPTY_INSTRUCTION:
                node->suffix = token_end_of_instruction;
                break;
            case Node_Kind_LITERAL:
            case Node_Kind_FUNCTION:
                break;
            default:
                NDBL_UNREACHABLE("Unexpected Node_Kind: %i\n", params.kind );
        }
    }
}

void node_init_as_empty_instruction(Node* node)
{
    Node_Init_Params params{};
    params.kind = Node_Kind_EMPTY_INSTRUCTION;
    
    node_init(node, params);
}

void node_init_as_for(Node* node)
{
    Node_Init_Params params{};
    params.kind = Node_Kind_FOR_LOOP;
    
    node_init(node, params);
}

void node_init_as_function(Node* node, const Type_Descriptor* type_descriptor)
{
    Node_Init_Params params{};
    params.kind            = Node_Kind_FUNCTION;
    params.type_descriptor = type_descriptor;

    node_init(node, params);
}

void node_init_as_if(Node* node)
{
    Node_Init_Params params{};
    params.kind = Node_Kind_IF_ELSE;

    node_init(node, params);
}

void node_init_as_literal(Node* node, const Type_Descriptor* type_descriptor)
{
    Node_Init_Params params{};
    params.kind            = Node_Kind_LITERAL;
    params.type_descriptor = type_descriptor;

    node_init(node, params);
}

void node_init_as_operator(Node* node, const Type_Descriptor* type_descriptor)
{
    Node_Init_Params params{};
    params.kind            = Node_Kind_OPERATOR;
    params.type_descriptor = type_descriptor;

    node_init(node, params);
}

void node_init_as_scope(Node* node)
{
    Node_Init_Params params{};
    params.kind = Node_Kind_SCOPE;
    
    node_init(node, params);
}

void node_init_as_root_scope(Node* node)
{
    Node_Init_Params params{};
    params.kind          = Node_Kind_SCOPE;
    params.scope_is_root = true;

    node_init(node, params);
}

void node_init_as_variable_ref(Node* node)
{
    Node_Init_Params params{};
    params.kind = Node_Kind_VARIABLE_REF;
    
    node_init(node, params);
}

void node_init_as_variable(Node* node, const Type_Descriptor* type_descriptor, const bdc::String identifier, bool with_assignment)
{
    Node_Init_Params params{};
    params.kind                     = Node_Kind_VARIABLE;
    params.type_descriptor          = type_descriptor;
    params.name                     = identifier.c_str();
    params.variable_with_assignment = with_assignment;

    node_init(node, params);
}

void node_init_as_while(Node* node)
{
    Node_Init_Params params{};
    params.kind = Node_Kind_WHILE_LOOP;
    
    node_init(node, params);
}

void node_deinit(Node* node)
{
    NDBL_ASSERT(node != nullptr);

    string_release(node->name);

    // deinit all props
    for(auto& prop : node->props )
    {
        property_deinit(prop);
        memory_delete(prop);
    }
    
    // destroy all props
    array_release(node->props);

    // clear the index
    hashmap_release(node->props_by_name);

    // deinit all slots
    for(auto& slot : node->slots )
    {
        node_slot_deinit(slot);
        memory_delete(slot);
    }
    // destroy all the slots
    array_release(node->slots);

    // clean the index
    node_deinit_component(&node->component, to_component_type(node->kind) );

    if( node->view )
    {
        // pass
        
        // Note: this is not the responsibility of a Node to free its view. The Graph_View adds Node_Views to Nodes, he handle that.
    }

    if( node->internal_scope )
    {
        scope_deinit(node->internal_scope);
        memory_delete(node->internal_scope);
        node->internal_scope = nullptr;
    }

    node->signal_deinit.emit();
}

Node* new_node(const Node_Init_Params& params)
{
    Node* node = memory_new<Node>();
    node_init(node, params);
    return node;
}

void node_init_component(Node::Component* component, Node::Component_Kind kind) 
{
    switch (kind)
    {
        case Node::Component_Kind_NULL:           break;
        case Node::Component_Kind_BRANCHING:      new (&component->branching)   Node::Branching_Component();    break;
        case Node::Component_Kind_LITERAL:        new (&component->literal)     Node::Literal_Component();      break;
        case Node::Component_Kind_INVOKABLE:      new (&component->invokable)   Node::Invokable_Component();    break;
        case Node::Component_Kind_VARIABLE_REF:   new (&component->variableref) Node::Variable_Ref_Component(); break;
        case Node::Component_Kind_VARIABLE:       new (&component->variable)    Node::Variable_Component();     break;
        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            NDBL_UNREACHABLE("Unhandled Component_Kind (value: %i)\n", kind);
    }
}

void node_deinit_component(Node::Component* component, Node::Component_Kind kind)
{
    switch (kind)
    {
        case Node::Component_Kind_NULL:           break;
        case Node::Component_Kind_BRANCHING:      component->branching.~Branching_Component();       break;
        case Node::Component_Kind_LITERAL:        component->literal.~Literal_Component();           break;
        case Node::Component_Kind_INVOKABLE:      component->invokable.~Invokable_Component();       break;
        case Node::Component_Kind_VARIABLE_REF:   component->variableref.~Variable_Ref_Component();  break;
        case Node::Component_Kind_VARIABLE:       component->variable.~Variable_Component();         break;
        default:
            // If it breaks here, that's because a new type has been added but this function does not take it in account.
            NDBL_UNREACHABLE("Unhandled Component_Kind (value: %i)\n", kind);
    }

    component->kind = 0;
}

Node::Component_Kind to_component_type(Node_Kind type)
{
    switch (type)
    {
        case Node_Kind_IF_ELSE:    [[fallthrough]];     
        case Node_Kind_WHILE_LOOP: [[fallthrough]];
        case Node_Kind_FOR_LOOP:
            return Node::Component_Kind_BRANCHING;

        case Node_Kind_LITERAL:
            return Node::Component_Kind_LITERAL;           

        case Node_Kind_OPERATOR: [[fallthrough]];
        case Node_Kind_FUNCTION:
            return Node::Component_Kind_INVOKABLE;

        case Node_Kind_VARIABLE_REF:
            return Node::Component_Kind_VARIABLE_REF;

        case Node_Kind_VARIABLE:
            return Node::Component_Kind_VARIABLE;

        default:
            // Those types do not have a dedicated data struct in the union
            return Node::Component_Kind_NULL;
    }
}

const Type_Descriptor* node_get_connected_function_type(const Node* node, const String& property_name)
{
    const Node_Slot* slot = node_find_slot_by_property_name(node, property_name, Node_Slot::Flag_INPUT );
    NDBL_ASSERT(slot!= nullptr, "Unable to find input slot for this property name");
    const Node_Slot* adjacent_slot = slot->first_adjacent();

    if ( adjacent_slot )
        if ( node_is_invokable(adjacent_slot->node) )
            return &adjacent_slot->node->component.invokable.type;

    return nullptr;
}

const Node_Slot* node_find_slot_by_property_name(const Node* node, const String& property_name, Node_Slot::Flags desired_way)
{
    const Node_Property* property = node_find_prop_by_name(node, property_name);
    if( property )
    {
        return node_find_slot_by_property( node, property, desired_way );
    }
    return nullptr;
}

const Node_Slot* node_find_slot_at(const Node* node, Node_Slot::Flags flags, size_t position)
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

Node_Slot* node_find_slot_by_property_type(const Node* node, Node_Slot::Flags flags, const Type_Descriptor* type)
{
    for(Node_Slot* slot : node_filter_slots(node, flags) )
    {
        if( type_is_implicitly_convertible(slot->property->type, type ) )
        {
            return slot;
        }
    }
    return nullptr;
}

void Node::handle_slot_change(Node_Slot::Event event, Node_Slot* slot)
{
    //
    // I was previously clearing some cache here, but I got issues with it and dediced to remove it until perf issues comes.
    //
}

void node_set_name(Node* node, const String& new_name)
{
    string_release(node->name);
    node->name = string_copy(new_name);
    node->signal_name_change.emit(node->name);
}

Node_Slot* node_add_slot(Node* node, Node_Property* property, Node_Slot::Flags flags, size_t capacity, size_t position)
{
    NDBL_ASSERT( property != nullptr );
    NDBL_ASSERT( property->node == node );
    if ( (flags & Node_Slot::Flag_FLOW_OUT) == Node_Slot::Flag_FLOW_OUT)
    {
        NDBL_ASSERT( capacity == 1, "Node_Slot::Flag_FLOW_OUT can only have a capacity of 1" );
    }

    Node_Slot* slot = memory_new<Node_Slot>();
    node_slot_init(slot, flags, capacity, position);
    slot->node      = node;
    slot->property  = property;

    array_append( node->slots, slot );
    array_append( property->slots, slot );

    // listen to events to clear cache
    slot->signal_change.connect<&Node::handle_slot_change>(node);

    return slot;
}

Array<Node_Slot*> node_filter_adjacent_slots(const Node* node, Node_Slot::Flags flags )
{
    Resizable_Array<Node_Slot*> result;
    result.allocator = &temp_allocator;

    for(Node_Slot* slot : node_filter_slots(node, flags))
        for( Node_Slot* slot : slot->adjacent )
            array_append( result, slot );

    return result;
}

bool node_has_input_connected(const Node* node, const Node_Property* property )
{
    const Node_Slot* slot = node_find_slot_by_property(node, property, Node_Slot::Flag_INPUT );
    return slot && slot->adjacent.size > 0;
}

const Node_Slot* node_find_slot_by_property(const Node* node, const Node_Property* prop, Node_Slot::Flags flags)
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

Node_Slot* node_find_adjacent_at(const Node* node, Node_Slot::Flags _flags, size_t _index )
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

Array<Node_Slot*> node_filter_slots(const Node* node, Node_Slot::Flags flags)
{
    const auto if_has_flags = [flags](const Node_Slot* _slot)
    {
        NDBL_ASSERT_DEBUG_ONLY(_slot != nullptr);
        return HAS_FLAGS(_slot->flags, flags);
    };
    return node_filter_slots(node, if_has_flags);
}

Array<Node_Slot*> node_filter_slots(const Node* node, const std::function<bool(const Node_Slot*)>& predicate)
{
    Resizable_Array<Node_Slot*> result;
    result.allocator = &temp_allocator;
    
    for(Node_Slot* slot : node->slots)
        if( predicate(slot) )
            array_append(result, slot);

    return result;
}

Node_Slot* Node::value_out()
{
    return const_cast<Node_Slot*>( node_find_slot_by_property(this, value, Node_Slot::Flag_OUTPUT ) );
}

const Node_Slot* Node::value_out() const
{
    return node_find_slot_by_property(this, value, Node_Slot::Flag_OUTPUT );
}

Node_Slot* Node::value_in()
{
    return const_cast<Node_Slot*>( node_find_slot_by_property(this, value, Node_Slot::Flag_INPUT ) );
}

const Node_Slot* Node::value_in() const
{
    return node_find_slot_by_property(this, value, Node_Slot::Flag_INPUT );
}

Node_Slot* Node::flow_enter()
{
    auto* const_this = const_cast<const Node*>(this);
    return const_cast<Node_Slot*>( const_this->flow_enter());
}

const Node_Slot* Node::flow_enter() const
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

Node_Slot* Node::flow_out()
{
    auto* const_this = const_cast<const Node*>(this);
    return const_cast<Node_Slot*>( const_this->flow_out());
}

const Node_Slot* Node::flow_out() const
{
    for(u32_t i = 0; i < value->slots.size; ++i )
        if( HAS_FLAGS(value->slots[i]->flags, Node_Slot::Flag_FLOW_OUT) )
            if (!HAS_FLAGS(value->slots[i]->flags, Node_Slot::Flag_IS_INTERNAL) ) // branches (internal) are specific flow_out, we don't want to grab them here
                return value->slots[i];
    return nullptr;
}

Node_Slot* Node::flow_in()
{
    return const_cast<Node_Slot*>( node_find_slot_by_property(this, value, Node_Slot::Flag_FLOW_IN ) );
}

const Node_Slot* Node::flow_in() const
{
    return node_find_slot_by_property(this, value, Node_Slot::Flag_FLOW_IN );
}

void node_init_internal_scope(Node* node)
{
    NDBL_ASSERT( node->internal_scope == nullptr, "Can't call init_internal_scope() more than once");
    NDBL_ASSERT( node->scope == nullptr, "Must be initialized prior to reset_parent()");

    auto* scope = memory_new<Scope>();
    scope->name = "Internal Scope";
    scope->node = node;

    node->internal_scope = scope;
}

bool node_has_flow_adjacent(const Node* node)
{
    return node->flow_inputs().size != 0 || node->flow_outputs().size != 0;
}

bool node_is_expression(const Node* node)
{
    return node->inputs().size != 0;
}

void node_reset_scope(Node* node, Scope* scope)
{
    if( node->scope == scope )
    {
        return;
    }

    if( node->scope )
    {
        node->flags |= Node_Flag_WAS_IN_A_SCOPE_ONCE;
    }
    node->scope = scope;

    if ( node->internal_scope != nullptr )
    {
        scope_reset_parent( node->internal_scope, scope );
    }
}

bool node_has_prop(const Node* node, const String& name)
{
    return hashmap_find(node->props_by_name, string_hash(name));
}

Node_Property* node_add_prop(Node* node, const Type_Descriptor* type, const String name, Node_Property::Flags flags )
{
    // guards
    NDBL_ASSERT(!node_has_prop(node, name), "Property name already used");

    // create
    auto* new_property = memory_new<Node_Property>(); // TODO: use a static-sized array with a given limit (ex: 10 props)
    property_init(new_property, node, type, flags, name);

    // register / index
    array_append(node->props, new_property);
    auto result = hashmap_add(node->props_by_name, string_hash(new_property->name), new_property);
    NDBL_ASSERT(result.ok);

    return new_property;
}

const Node_Property* node_find_first_prop(const Node* node, Node_Property::Flags _flags, const Type_Descriptor *_type)
{
    FOR_EACH_HASHMAP_VALUE(property, node->props_by_name)        
    {
        if( type_is_implicitly_convertible(property->type, _type) && ( HAS_FLAGS(property->flags, _flags ) ) )
        {
            return property;
        }
    }
    return nullptr;
}

const Node_Property* node_find_prop_by_name(const Node* node, const String& name)
{
    bdc::Result<Node_Property*> result = hashmap_find(node->props_by_name, string_hash(name) );

    if( result.ok )
    {
        return result.data;
    }

    NDBL_ASSERT(false);
    return nullptr;
}

void node_variable_ref_set_variable(Node* node, Node* variable_node)
{
    NDBL_ASSERT_DEBUG_ONLY(variable_node != nullptr);
    NDBL_ASSERT( node->component.variableref.variable_node == nullptr, "Can't call twice");

    node->component.variableref.variable_node = variable_node;

    property_set_type(node->value, node_variable_type(variable_node) );
    const String identifier = node_get_identifier(variable_node);
    token_replace_word( node->value->token, identifier );

    // bind signals
    node->component.variableref.variable_node->signal_name_change.connect< &node_variable_ref_handle_name_change>(node);
    node->component.variableref.variable_node->signal_deinit.connect<&node_variable_ref_clear_variable>(node);
}

void node_variable_ref_clear_variable(Node* node)
{
    Node* variable_node = node->component.variableref.variable_node; 
    if ( variable_node == nullptr )
        return;

    // unbind signals
    variable_node->signal_name_change.disconnect();
    variable_node->signal_deinit.disconnect();
    variable_node = nullptr;
}

void node_variable_ref_handle_name_change(Node* node, const String& name)
{
    token_replace_word( node->value->token, name );
}

void node_init_branches(Node* node, size_t branch_count)
{
    NDBL_ASSERT( node->component.kind == Node::Component_Kind_BRANCHING, "Node does not have a switch behavior" );
    NDBL_ASSERT( 1 < branch_count && branch_count <= Node::Branching_Component::BRANCH_MAX, "branch_count is out of range");

    node->component.branching.branch_count = branch_count;

    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);      // accepts N inputs
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT , 1); // accepts 0 or 1 output

    // add 1 slot per branch
    for(size_t branch = 0; branch < branch_count; ++branch )
    {
        node->component.branching.branch_slots[branch] = node_add_slot(node, node->value, Node_Slot::Flag_FLOW_ENTER, 1, branch);
    }

    // add 1 condition per branch except for the default branch
    for(size_t branch = 1; branch < branch_count; ++branch )
    {
        auto condition_property = node_add_prop<any>(node, CONDITION_PROPERTY);
        node->component.branching.condition_in_slots[branch-1]  = node_add_slot(node, condition_property, Node_Slot::Flag_INPUT, 1, branch);
    }
}

void node_init_as_return(Node* node, const Type_Descriptor* type_descriptor)
{
    Node_Init_Params params{};
    params.kind            = Node_Kind_RETURN;
    params.type_descriptor = type_descriptor;

    node_init(node, params);
}


Array<Node*> node_get_adjacent_nodes(const Node* node, Node_Slot::Flags flags)
{
    Resizable_Array<Node*> result;
    result.allocator = &temp_allocator;
    
    for ( Node_Slot* slot : node_filter_slots(node, flags ) )
    {
        for( const Node_Slot* adjacent : slot->adjacent )
        {
            array_append(result, adjacent->node );
        }
    }
    return result;
}

Node* node_adjacent_node_at(const Node* node, Node_Slot::Flags flags, u8_t pos)
{
    if ( Node_Slot* adjacent_slot = node_find_adjacent_at(node, flags, pos ) )
    {
        return adjacent_slot->node;
    }
    return {};
}

bool node_is_instruction(const Node* node)
{
    if ( node_is_connected_to_codeflow(node) )
        return true;
    if ( node->kind == Node_Kind_VARIABLE )
        return true;
    return false;
}

bool node_is_connected_to_codeflow(const Node *node)
{
    if (node->flow_inputs().size != 0)
        return true;
    if (node->flow_outputs().size != 0)
        return true;
    return false;
}

bool node_could_be_instruction(const Node* node)
{
    // TODO: handle case where a variable has inputs/outputs but not connected to the code flow
    return node_slot_count(node, Node_Slot::Flag_TYPE_FLOW) > 0 && node->inputs().size == 0 && node->outputs().size == 0;
}

bool node_is_unary_operator(const Node* node)
{
    if (node->kind == Node_Kind_OPERATOR )
        if (node->component.invokable.type.function.args.size == 1 )
            return true;
    return false;
}

bool node_is_binary_operator(const Node* node)
{
    if (node->kind == Node_Kind_OPERATOR )
        if (node->component.invokable.type.function.args.size == 2 )
            return true;
    return false;
}

bool node_is_conditional(const Node* node)
{
    switch ( node->kind )
    {
        case Node_Kind_FOR_LOOP:
        case Node_Kind_WHILE_LOOP:
        case Node_Kind_IF_ELSE:
            return true;
        default:
            return false;
    };
}

bool node_is_output_node_in_expression(const Node* input_node, const Node* output_node)
{
#ifdef NDBL_DEBUG
    NDBL_ASSERT(input_node);
    NDBL_ASSERT(output_node);
    const bool is_an_output = std::find(input_node->outputs().begin(), input_node->outputs().end(), output_node) != input_node->outputs().end();
    NDBL_ASSERT(is_an_output);
#endif

    if ( node_is_instruction(input_node ) )
    {
        if ( input_node->kind == Node_Kind_VARIABLE )
        {
            const Node_Slot* declaration_out = input_node->component.variable.decl_out;
            return declaration_out->first_adjacent_node() == output_node;
        }
        return false;
    }
    return input_node->outputs()[0] == output_node;
}

bool node_is_initialized(const Node* node)
{
    return node != nullptr && (node->flags & Node_Flag_IS_INITIALIZED);
}

} // namespace ndbl
