#pragma once

#include <string>
#include <array>
#include <vector>

#include "tools/core/Asserts.h"
#include "tools/core/Component.h"
#include "tools/core/reflection/Type_Descriptor.h"

#include "Node_Property.h"
#include "Node_Slot.h"

namespace ndbl
{
    // forward declarations
    class Graph;
    class Scope;
    class Node;
    class Node_Slot;
    
    typedef int Node_Type;
    enum Node_Type_
    {
        // enum is used to index arrays, must start at 0 with no gaps

        Node_Type_NULL                  =  0,
        Node_Type_SCOPE                 =  1,
        Node_Type_ROOT                  =  2,
        Node_Type_IF_ELSE               =  3,
        Node_Type_FOR_LOOP              =  4,
        Node_Type_WHILE_LOOP            =  5,
        Node_Type_VARIABLE              =  6,
        Node_Type_VARIABLE_REF          =  7,
        Node_Type_LITERAL               =  8,
        Node_Type_FUNCTION              =  9,
        Node_Type_OPERATOR              = 10,
        Node_Type_EMPTY_INSTRUCTION     = 11,
        Node_Type_RETURN                = 12,

        Node_Type_COUNT,
    };

    typedef size_t Branch;
    enum Branch_ : size_t
    {
        Branch_FALSE = 0,
        Branch_TRUE  = 1,
    };

    typedef int VariableFlags;
    enum VariableFlags_
    {
        VariableFlag_NONE        = 0,
        VariableFlag_DECLARED    = 1 << 0,
        VariableFlag_INITIALIZED = 1 << 1,
        VariableFlag_ALL         = ~VariableFlag_NONE
    };

    typedef int Node_Flags;
    enum Node_Flag_
    {
        Node_Flag_NONE                = 0,
        Node_Flag_IS_DIRTY            = 1 << 0,
        Node_Flag_WAS_IN_A_SCOPE_ONCE = 1 << 1,
        Node_Flag_MUST_BE_DELETED     = 1 << 2,
        Node_Flag_IS_INITIALIZED      = 1 << 3,
        Node_Flag_ALL                 = ~Node_Flag_NONE,
        Node_Flag_DEFAULT             = Node_Flag_NONE,
    };

    struct Adjacent_Nodes /* Cached */
    {
        const Node*                     node;
        mutable std::unordered_map<
            Node_Slot::Flags,
            std::vector<Node*>>         cache;

        Adjacent_Nodes(const Node* _node): node(_node) {}
    };

    const std::vector<Node*>& adjacent_nodes_get(const Adjacent_Nodes*, Node_Slot::Flags);
    
    struct Node_State
    {
        Node_Type                   type;
        bool                        user_created;
        tools::Function_Descriptor* function_descriptor; // TODO: this has to be serializable!
    };

    struct Node
	{
        DECLARE_REFLECT        
        friend class Scope;
        friend class Graph;

        Node();
        ~Node();
        Node(const Node&) = delete;
        Node(Node&&)      = delete;

        struct Switch_Behavior_State
        {
            // Handle any conditionnal structure, for now it only handle 2 branches, but it will be modified to have N branches (like a switch)

            static constexpr size_t                 BRANCH_MAX          = 2;
            Token                                   branch_prefix       = {Token_Type::ignore}; // e.g. if|for|while
            Token                                   branch_suffix       = {Token_Type::ignore}; // e.g. else
            size_t                                  branch_count        = 0;
            std::array<Node_Slot*, BRANCH_MAX>      branch_slots        = {};
            std::array<Node_Slot*, BRANCH_MAX - 1>  condition_in_slots  = {};            
            Node_Slot*                              initialization_slot = {nullptr};
            Node_Slot*                              iteration_slot      = {nullptr};

            Node_Slot*          branch_out(Branch branch)                       { ASSERT(branch < branch_count); return branch_slots[branch]; }
            const Node_Slot*    branch_out(Branch branch) const                 { ASSERT(branch < branch_count); return branch_slots[branch]; }
            const Node*         condition(Branch branch = Branch_TRUE) const    { ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]->first_adjacent_node(); }
            Node*               condition(Branch branch = Branch_TRUE)          { ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]->first_adjacent_node(); }
            const Node_Slot*    condition_in(Branch branch = Branch_TRUE) const { ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]; }
            Node_Slot*          condition_in(Branch branch = Branch_TRUE)       { ASSERT(Branch_FALSE < branch && branch < branch_count); return condition_in_slots[branch - 1]; }    
        };

        struct Invokable_State
        {
            Token                                   identifier_token = {Token_Type::identifier };
            tools::Function_Descriptor              func_type; // not owned
            tools::Inline_Vector8<Node_Slot*>       argument_slots;
            tools::Inline_Vector8<Node_Property*>   argument_props;

            Node_Slot*                              lvalue_in() const { return argument_slots[0]; }
            Node_Slot*                              rvalue_in() const { return argument_slots[1]; }
        };

        struct Variable_State
        { 
            Token         type_token        = {Token_Type::keyword_unknown }; // [int] var  =
            Token         operator_token    = {Token_Type::operator_ };       //  int  var [=]
            VariableFlags flags             = VariableFlag_NONE;
            Node_Slot*    decl_out          = nullptr;
            Node_Slot*    ref_out           = nullptr;
        };

        struct Variable_Ref_State
        {
            Node* variable_node = nullptr;
        };

        struct Literal_State
        {
            Token                           token = {Token_Type::literal_any};
            const tools::Type_Descriptor*   type  = nullptr;
        };

        // members are sorted from large to small
            
        union // depends on this->type
        {            
            Switch_Behavior_State               switch_data;                // check type before use!
            Invokable_State                     invokable_data;             // check type before use!
            Variable_State                      variable_data;              // check type before use!
            Variable_Ref_State                  variableref_data;           // check type before use!
            Literal_State                       literal_data;               // check type before use!
        };

        tools::Simple_Signal                    signal_deinit     = {}; // emit once component_deinit() has been called
        tools::Signal<void(const std::string&)> signal_name_change  = {};   
        std::vector<Node_Property*>             props               = {}; // TODO: size-fixed array?
        std::map<std::string, Node_Property*>   props_by_name       = {};
        std::vector<Node_Slot*>                 slots               = {}; // TODO: size-fixed array?
        std::unordered_map<
        const Node_Property*,
        std::vector<Node_Slot*>>                slots_by_prop       = {}; // TODO: if we are sure a property has a fixed index, we could use a vector instead
        Adjacent_Nodes                          adjacent_nodes;
        tools::Component_Bag<Node>              component_bag;
        std::string                             name                = {};
        Token                                   suffix              = Token{};
        Graph*                                  graph               = nullptr;
        Node_Property*                          value               = nullptr; // this Node_Property
        Scope*                                  scope               = nullptr; 
        Scope*                                  internal_scope      = nullptr;
        Node_Type                               type                = Node_Type_NULL;
        Node_Flags                              flags               = Node_Flag_IS_DIRTY;
            
        bool                                    has_flags(Node_Flags _flags)const { return (flags & _flags) == _flags; };
        void                                    set_flags(Node_Flags _flags) { flags |= _flags; }
        void                                    clear_flags(Node_Flags _flags = Node_Flag_ALL) { flags &= ~_flags; }
        
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
        const std::vector<Node*>&               inputs() const       { return adjacent_nodes_get(&this->adjacent_nodes, Node_Slot::Flag_INPUT); }
        const std::vector<Node*>&               outputs() const      { return adjacent_nodes_get(&this->adjacent_nodes, Node_Slot::Flag_OUTPUT); }
        const std::vector<Node*>&               flow_inputs() const  { return adjacent_nodes_get(&this->adjacent_nodes, Node_Slot::Flag_FLOW_IN); }
        const std::vector<Node*>&               flow_outputs() const { return adjacent_nodes_get(&this->adjacent_nodes, Node_Slot::Flag_FLOW_OUT); }
        void                                    handle_slot_change(Node_Slot::Event, Node_Slot*);        
    };

    void                    node_init(Node*, Node_Type, const std::string& name);
    void                    node_init_as_empty_instruction(Node*);  // TODO: remove this, move code into node_init()
    void                    node_init_as_while_loop(Node*);         // TODO:  (same)
    void                    node_init_as_for_loop(Node*);           // TODO:  (same)
    void                    node_init_as_cond_struct(Node*);        // TODO:  (same)
    void                    node_init_as_scope(Node*);              // TODO:  (same)
    void                    node_init_as_root_scope(Node*);         // TODO:  (same)
    void                    node_init_as_variable_ref(Node*);       // TODO:  (same)
    void                    node_init_as_return(Node* node, const tools::Type_Descriptor* = nullptr);
    void                    node_init_as_invokable(Node*, const tools::Function_Descriptor*, Node_Type = Node_Type_FUNCTION);
    void                    node_init_as_variable(Node*, const tools::Type_Descriptor*, const char* identifier);
    void                    node_init_as_literal(Node*, const tools::Type_Descriptor*);
    void                    node_init_internal_scope(Node*);
    void                    node_init_branches(Node*, size_t branch_count);
    void                    node_deinit(Node*);
    bool                    node_update(Node*);
    void                    node_reset_scope(Node*, Scope*);
    void                    node_variable_ref_clear_variable(Node*);
    void                    node_variable_ref_set_variable(Node*, Node* /* variable_node */);
    void                    node_variable_ref_handle_name_change(Node*, const std::string& /*name*/);
    inline const tools::Type_Descriptor* node_variable_type(const Node* node ) { return node->value->type; }
    inline const Token&     node_get_identifier_token(const Node* node) { return node->value->token; }
    inline Token&           node_get_identifier_token(Node* node) { return node->value->token; }
    inline void             node_set_identifier_token(Node* node, const Token& tok) { node->value->token = tok; }
    inline std::string      node_get_identifier(const Node* node) { return node_get_identifier_token(node).word_to_string(); }
    inline void             node_set_name(Node* node, const std::string& _name) { node->name = _name; node->signal_name_change.emit(node->name); }
    
    // Slot-related
    Node_Slot*              node_add_slot(Node*, Node_Property *, Node_Slot::Flags, size_t limit_capacity = 0, size_t _position = 0);
    bool                    node_has_flow_adjacent(const Node*);
    std::vector<Node_Slot*> node_filter_slots(const Node*, Node_Slot::Flags);
    std::vector<Node_Slot*> node_filter_slots(const Node*, const std::function<bool(const Node_Slot*)>& predicate);
    std::vector<Node_Slot*> node_filter_adjacent_slots(const Node*, Node_Slot::Flags);
    inline size_t           node_adjacent_slot_count(const Node* node, Node_Slot::Flags flags) { return node_filter_adjacent_slots(node, flags).size(); }
    inline size_t           node_slot_count(const Node* node, Node_Slot::Flags flags) { return node_filter_slots(node, flags).size(); }
    const Node_Slot*        node_find_slot_at(const Node*, Node_Slot::Flags, size_t _position ); // implicitly DEFAULT_PROPERTY's slot
    inline Node_Slot*       node_find_slot_at(Node* node, Node_Slot::Flags flags, size_t pos ) { return const_cast<Node_Slot*>( node_find_slot_at(const_cast<const Node*>(node), flags, pos)); } // implicitly DEFAULT_PROPERTY's slot
    const Node_Slot*        node_find_slot_by_property_name(const Node*, const char* name, Node_Slot::Flags );
    inline Node_Slot*       node_find_slot_by_property_name(Node* node, const char* name, Node_Slot::Flags flags) { return const_cast<Node_Slot*>( node_find_slot_by_property_name(const_cast<const Node*>(node), name, flags) ); };
    Node_Slot*              node_find_slot_by_property_type(const Node*, Node_Slot::Flags _way, const tools::Type_Descriptor *_type);
    const Node_Slot*        node_find_slot_by_property(const Node*, const Node_Property*, Node_Slot::Flags );
    inline Node_Slot*       node_find_slot_by_property(Node* node, const Node_Property* prop, Node_Slot::Flags flags ) { return const_cast<Node_Slot*>( node_find_slot_by_property(const_cast<const Node*>(node), prop, flags ) ); }
    inline const Node_Slot* node_find_slot(const Node* node, Node_Slot::Flags flags) { return node_find_slot_by_property(node, node->value, flags ); }// implicitly DEFAULT_PROPERTY's slot
    inline Node_Slot*       node_find_slot(Node* node, Node_Slot::Flags flags) { return node_find_slot_by_property(node, node->value, flags ); }// implicitly DEFAULT_PROPERTY's slot
    inline Node_Slot*       node_find_adjacent_at(const Node*, Node_Slot::Flags, size_t _index );

    // Property-related
    Node_Property*          node_add_prop(Node*, const tools::Type_Descriptor*, const char* name, Node_Property::Flags = Node_Property::Flag_NONE);
    template<typename T>    
    Node_Property*          node_add_prop(Node* node, const char* name, Node_Property::Flags flags = Node_Property::Flag_NONE ) { return node_add_prop(node, tools::type::get<T>(), name, flags); }
    bool                    node_has_input_connected(const Node*, const Node_Property*);
    bool                    node_has_prop(const Node*, const char*);
    const Node_Property*    node_find_prop_by_name(const Node*, const char* name);
    inline Node_Property*   node_find_prop_by_name(Node* node, const char* name) { return const_cast<Node_Property*>( node_find_prop_by_name(const_cast<const Node*>(node), name) );}
    const Node_Property*    node_find_first_prop(const Node*, Node_Property::Flags, const tools::Type_Descriptor* );
    inline Node_Property*   node_find_first_prop(Node* node, Node_Property::Flags flags, const tools::Type_Descriptor* type ) { return const_cast<Node_Property*>( node_find_first_prop(const_cast<const Node*>(node), flags, type) );}
    const tools::Function_Descriptor* node_get_connected_function_type(const Node*, const char *property_name); //

    // Misc.

    Node*                   node_adjacent_node_at(const Node*, Node_Slot::Flags, u8_t pos);
    bool                    node_could_be_instruction(const Node*);
    bool                    node_has_switch_behavior(const Node*);
    std::vector<Node*>      node_get_adjacent_nodes(const Node*, Node_Slot::Flags);
    bool                    node_is_instruction(const Node*);
    bool                    node_is_unary_operator(const Node*);
    bool                    node_is_binary_operator(const Node*);
    bool                    node_is_conditional(const Node*);
    bool                    node_is_connected_to_codeflow(const Node *node);
    bool                    node_is_output_node_in_expression(const Node* input_node, const Node* output_node);
    bool                    node_is_initialized(const Node* node);
    bool                    node_is_expression(const Node*);
    inline bool             node_is_invokable(const Node* node)     { return node->type == Node_Type_OPERATOR || node->type == Node_Type_FUNCTION; }
    inline bool             node_is_orphan(const Node* node)        { return node->scope == nullptr; }

    template<typename Component_Type>
    inline Component_Type* node_component(Node* node) { return tools::componentbag_get<Component_Type>(&node->component_bag); }
}
