#pragma once

#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include "bdc/Hash_Map.hpp"
#include "bdc/String_Hash.hpp"
#include "Asserts.h"
#include "reflection/Type_Descriptor.h"
#include "Node_Property.h"
#include "Node_Slot.h" // Note: could we forward declare this?

namespace ndbl
{
    // forward declarations
    struct Graph;
    struct Scope;
    struct Node_View;
    
    typedef u32_t Node_ID;
    typedef u32_t Slot_Node_ID;
    typedef u32_t Scope_ID;

    typedef int Node_Kind;
    enum Node_Kind_
    {
        // enum is used to index arrays, must start at 0 with no gaps
        // values are provided for debug reference.

        Node_Kind_NULL                  =  0,
        Node_Kind_SCOPE                 =  1,
        Node_Kind_IF_ELSE               =  3,
        Node_Kind_FOR_LOOP              =  4,
        Node_Kind_WHILE_LOOP            =  5,
        Node_Kind_VARIABLE              =  6,
        Node_Kind_VARIABLE_REF          =  7,
        Node_Kind_LITERAL               =  8,
        Node_Kind_FUNCTION              =  9,
        Node_Kind_OPERATOR              = 10,
        Node_Kind_EMPTY_INSTRUCTION     = 11,
        Node_Kind_RETURN                = 12,

        Node_Kind_COUNT,
    };

    typedef size_t Branch;
    enum Branch_ : size_t
    {
        Branch_FALSE = 0,
        Branch_TRUE  = 1,
    };

    typedef int Node_Flags;
    enum Node_Flag_
    {
        Node_Flag_NONE                = 0,
        Node_Flag_WAS_IN_A_SCOPE_ONCE = 1 << 1,
        Node_Flag_MARKED_FOR_DELETION = 1 << 2,
        Node_Flag_IS_INITIALIZED      = 1 << 3,
        Node_Flag_ALL                 = ~Node_Flag_NONE,
        Node_Flag_DEFAULT             = Node_Flag_NONE,
    };

    struct Node
	{
        DECLARE_REFLECT        

        typedef int Component_Kind;
        enum Component_Kind_
        {
            Component_Kind_NULL = 0,
            Component_Kind_BRANCHING,
            Component_Kind_INVOKABLE,
            Component_Kind_VARIABLE,
            Component_Kind_VARIABLE_REF,
            Component_Kind_LITERAL
        };

        struct Branching_Component
        {
            // Handle any conditionnal structure, for now it only handle 2 branches, but it will be modified to have N branches (like a switch)

            static constexpr size_t BRANCH_MAX = 2;
            Component_Kind          kind = Component_Kind_BRANCHING;
            Token                   branch_prefix; // for if|for|while
            Token                   branch_suffix; // for else
            size_t                  branch_count;
            Node_Slot*              branch_slots[BRANCH_MAX];
            Node_Slot*              condition_in_slots[BRANCH_MAX - 1];            
            Node_Slot*              initialization_slot;
            Node_Slot*              iteration_slot;

            Node_Slot*              branch_out(Branch branch)                       { NDBL_ASSERT(branch < branch_count); return branch_slots[branch]; }
            const Node_Slot*        branch_out(Branch branch) const                 { NDBL_ASSERT(branch < branch_count); return branch_slots[branch]; }
            const Node*             condition(Branch branch = Branch_TRUE) const    { NDBL_ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]->first_adjacent_node(); }
            Node*                   condition(Branch branch = Branch_TRUE)          { NDBL_ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]->first_adjacent_node(); }
            const Node_Slot*        condition_in(Branch branch = Branch_TRUE) const { NDBL_ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]; }
            Node_Slot*              condition_in(Branch branch = Branch_TRUE)       { NDBL_ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]; }    
        };

        struct Invokable_Component
        {
            Component_Kind                          kind = Component_Kind_INVOKABLE;
            Token                                   identifier_token;
            Type_Descriptor                         type; // not owned
            bdc::Inlined_Array<Node_Slot*, 8>       argument_slots;
            bdc::Inlined_Array<Node_Property*, 8>   argument_props;

            Node_Slot*                              lvalue_in() const { return argument_slots[0]; }
            Node_Slot*                              rvalue_in() const { return argument_slots[1]; }
        };

        struct Variable_Component
        { 
            Component_Kind  kind = Component_Kind_VARIABLE;
            Token           type_token;       // [int] var  =
            Token           operator_token;   //  int  var [=]
            Node_Slot*      decl_out;
            Node_Slot*      ref_out;
        };

        struct Variable_Ref_Component
        {
            Component_Kind  kind = Component_Kind_VARIABLE_REF;
            Node*           variable_node;
        };

        struct Literal_Component
        {
            Component_Kind           kind = Component_Kind_LITERAL;
            Token                    token;
            const Type_Descriptor*   type;
        };

        union Component
        {          
            Component_Kind          kind = 0;  
            Branching_Component     branching;          // check kind before to use!
            Invokable_Component     invokable;          // check kind before to use!
            Variable_Component      variable;           // check kind before to use!
            Variable_Ref_Component  variableref;        // check kind before to use!
            Literal_Component       literal;            // check kind before to use!
            ~Component();
        };

        Node() {};
        ~Node() = default;
        
        Node_ID                                 id = {};
        Component                               component;
        Simple_Signal                           signal_deinit;
        Signal<void(const bdc::String&)>        signal_name_change;   
        bdc::Resizable_Array<Node_Property*>    props;
        bdc::Resizable_Array<Node_Slot*>        slots;
        bdc::Hash_Map<bdc::String_Hash, Node_Property*>   props_by_name;
        bdc::String                             name;
        Token                                   suffix;
        Graph*                                  graph = nullptr;
        Node_Property*                          value = nullptr; // equivalent of "this"
        Scope*                                  scope = nullptr; 
        Scope*                                  internal_scope = nullptr;
        Node_Kind                               kind  = 0;
        Node_Flags                              flags = 0;
        Node_View*                              view  = nullptr;

        Node_Slot*                              value_in();
        const Node_Slot*                        value_in() const;
        Node_Slot*                              value_out();
        const Node_Slot*                        value_out() const;
        Node_Slot*                              flow_in();
        const Node_Slot*                        flow_in() const;
        Node_Slot*                              flow_out();
        const Node_Slot*                        flow_out() const;
        Node_Slot*                              flow_enter();
        const Node_Slot*                        flow_enter() const;
        bdc::Array<Node*>                       inputs() const;
        bdc::Array<Node*>                       outputs() const;
        bdc::Array<Node*>                       flow_inputs() const;
        bdc::Array<Node*>                       flow_outputs() const;
        void                                    handle_slot_change(Node_Slot::Event, Node_Slot*);        
    };

    
    struct Node_Init_Params
    {
        Node_ID                 id;                 // optionnal, if null an automatic id will be generated (see node_generate_new_id()).
        Node_Kind               kind;
        bdc::String             name;
        Scope_ID                parent_scope_id;            // optionnal
        const Type_Descriptor*  type_descriptor;            // optionnal
        bool                    created_by_the_user;        // Note: could be in flags? But harder to debug..
        bool                    variable_with_assignment;   // Note: could be in flags? But harder to debug..
        bool                    scope_is_root;              // Note: could be in flags? But harder to debug..
    };

    Node*                   new_node(const Node_Init_Params&);
    void                    node_init(Node*, const Node_Init_Params& = {}); // generic initialization
    void                    node_init_as_empty_instruction(Node*);
    void                    node_init_as_for(Node*);
    void                    node_init_as_function(Node*, const Type_Descriptor*);
    void                    node_init_as_if(Node*);
    void                    node_init_as_literal(Node*, const Type_Descriptor*);
    void                    node_init_as_operator(Node*, const Type_Descriptor*);
    void                    node_init_as_return(Node*, const Type_Descriptor* = nullptr);
    void                    node_init_as_scope(Node*);
    void                    node_init_as_root_scope(Node*);
    void                    node_init_as_variable_ref(Node*);
    void                    node_init_as_variable(Node*, const Type_Descriptor*, const bdc::String identifier, bool with_assignment);
    void                    node_init_as_while(Node*);
    void                    node_init_internal_scope(Node*);
    void                    node_init_branches(Node*, size_t branch_count);
    void                    node_init_component(Node::Component*, Node::Component_Kind);
    void                    node_deinit(Node*);
    void                    node_deinit_component(Node::Component*, Node::Component_Kind);
    void                    node_reset_scope(Node*, Scope*);
    void                    node_variable_ref_clear_variable(Node*);
    void                    node_variable_ref_set_variable(Node*, Node* /* variable_node */);
    void                    node_variable_ref_handle_name_change(Node*, const bdc::String& /*name*/);
    inline const Type_Descriptor* node_variable_type(const Node* node ) { return node->value->type; }
    inline const Token&     node_get_identifier_token(const Node* node) { return node->value->token; }
    inline Token&           node_get_identifier_token(Node* node) { return node->value->token; }
    inline void             node_set_identifier_token(Node* node, const Token& tok) { node->value->token = tok; }
    inline bdc::String      node_get_identifier(const Node* node) { return token_word_view( node_get_identifier_token(node) ); }
    void                    node_set_name(Node* node, const bdc::String& _name);
    
    // Slot-related
    Node_Slot*              node_add_slot(Node*, Node_Property *, Node_Slot::Flags, size_t limit_capacity = 0, size_t _position = 0);
    bool                    node_has_flow_adjacent(const Node*);
    bdc::Array<Node_Slot*>  node_filter_slots(const Node*, Node_Slot::Flags);
    bdc::Array<Node_Slot*>  node_filter_slots(const Node*, const std::function<bool(const Node_Slot*)>& predicate);
    bdc::Array<Node_Slot*>  node_filter_adjacent_slots(const Node*, Node_Slot::Flags);
    inline size_t           node_adjacent_slot_count(const Node* node, Node_Slot::Flags flags) { return node_filter_adjacent_slots(node, flags).size; }
    inline size_t           node_slot_count(const Node* node, Node_Slot::Flags flags) { return node_filter_slots(node, flags).size; }
    const Node_Slot*        node_find_slot_at(const Node*, Node_Slot::Flags, size_t _position ); // implicitly DEFAULT_PROPERTY's slot
    inline Node_Slot*       node_find_slot_at(Node* node, Node_Slot::Flags flags, size_t pos ) { return const_cast<Node_Slot*>( node_find_slot_at(const_cast<const Node*>(node), flags, pos)); } // implicitly DEFAULT_PROPERTY's slot
    const Node_Slot*        node_find_slot_by_property_name(const Node*, const bdc::String& name, Node_Slot::Flags );
    inline Node_Slot*       node_find_slot_by_property_name(Node* node, const bdc::String& name, Node_Slot::Flags flags) { return const_cast<Node_Slot*>( node_find_slot_by_property_name(const_cast<const Node*>(node), name, flags) ); };
    Node_Slot*              node_find_slot_by_property_type(const Node*, Node_Slot::Flags _way, const Type_Descriptor *_type);
    const Node_Slot*        node_find_slot_by_property(const Node*, const Node_Property*, Node_Slot::Flags );
    inline Node_Slot*       node_find_slot_by_property(Node* node, const Node_Property* prop, Node_Slot::Flags flags ) { return const_cast<Node_Slot*>( node_find_slot_by_property(const_cast<const Node*>(node), prop, flags ) ); }
    inline const Node_Slot* node_find_slot(const Node* node, Node_Slot::Flags flags) { return node_find_slot_by_property(node, node->value, flags ); }// implicitly DEFAULT_PROPERTY's slot
    inline Node_Slot*       node_find_slot(Node* node, Node_Slot::Flags flags) { return node_find_slot_by_property(node, node->value, flags ); }// implicitly DEFAULT_PROPERTY's slot
    inline Node_Slot*       node_find_adjacent_at(const Node*, Node_Slot::Flags, size_t _index );

    // Property-related
    Node_Property*          node_add_prop(Node*, const Type_Descriptor*, const bdc::String name, Node_Property::Flags = Node_Property::Flag_NONE);
    template<typename T>    
    Node_Property*          node_add_prop(Node* node, const bdc::String& name, Node_Property::Flags flags = Node_Property::Flag_NONE ) { return node_add_prop(node, type_get<T>(), name, flags); }
    bool                    node_has_input_connected(const Node*, const Node_Property*);
    bool                    node_has_prop(const Node*, const bdc::String&);
    const Node_Property*    node_find_prop_by_name(const Node*, const bdc::String& name);
    inline Node_Property*   node_find_prop_by_name(Node* node, const bdc::String& name) { return const_cast<Node_Property*>( node_find_prop_by_name(const_cast<const Node*>(node), name) );}
    const Node_Property*    node_find_first_prop(const Node*, Node_Property::Flags, const Type_Descriptor* );
    inline Node_Property*   node_find_first_prop(Node* node, Node_Property::Flags flags, const Type_Descriptor* type ) { return const_cast<Node_Property*>( node_find_first_prop(const_cast<const Node*>(node), flags, type) );}
    const Type_Descriptor*  node_get_connected_function_type(const Node*, const bdc::String& property_name); //

    // Misc.

    Node_ID                 node_generate_new_id();

    bdc::Array<ndbl::Node*> node_get_adjacent_nodes(const Node*, Node_Slot::Flags); // may update cache and return result
    Node*                   node_adjacent_node_at(const Node*, Node_Slot::Flags, u8_t pos);
    bool                    node_could_be_instruction(const Node*);
    bool                    node_has_switch_behavior(const Node*);
    bool                    node_is_instruction(const Node*);
    bool                    node_is_unary_operator(const Node*);
    bool                    node_is_binary_operator(const Node*);
    bool                    node_is_conditional(const Node*);
    bool                    node_is_connected_to_codeflow(const Node *node);
    bool                    node_is_output_node_in_expression(const Node* input_node, const Node* output_node);
    bool                    node_is_initialized(const Node* node);
    bool                    node_is_expression(const Node*);
    inline bool             node_is_invokable(const Node* node)     { return node->kind == Node_Kind_OPERATOR || node->kind == Node_Kind_FUNCTION; }
    inline bool             node_is_orphan(const Node* node)        { return node->scope == nullptr; }
}
