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

        Node_Type_NULL = 0,
        Node_Type_SCOPE,
        Node_Type_ROOT,
        Node_Type_IF_ELSE,
        Node_Type_FOR_LOOP,
        Node_Type_WHILE_LOOP,
        Node_Type_VARIABLE,
        Node_Type_VARIABLE_REF,
        Node_Type_LITERAL,
        Node_Type_FUNCTION,
        Node_Type_OPERATOR,
        Node_Type_EMPTY_INSTRUCTION,

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
        Node_Flag_ALL                 = ~Node_Flag_NONE,
        Node_Flag_DEFAULT             = Node_Flag_NONE,
    };

    struct Node
	{
        DECLARE_REFLECT        
        friend class Scope;
        friend class Graph;

//===== INTERNAL STRUCTS ===============================================================================================

        struct Adjacent_Nodes_Cache
        {
            // Struct to get a list of nodes from given flags, caches the result

            explicit Adjacent_Nodes_Cache(const Node* node): _node(node) {}
            const std::vector<Node*>& get(Node_Slot::Flags) const;
            void set_dirty() { _cache.clear(); }
        private:
            const Node* _node;
            std::unordered_map<Node_Slot::Flags, std::vector<Node*>> _cache;
        };

        struct Switch_Behavior_State
        {
            // Handle any conditionnal structure, for now it only handle 2 branches, but it will be modified to have N branches (like a switch)

            static constexpr size_t                 BRANCH_MAX            = 2;
            Token                                   m_branch_prefix       = {Token_Type::ignore}; // e.g. if|for|while
            Token                                   m_branch_suffix       = {Token_Type::ignore}; // e.g. else
            size_t                                  m_branch_count        = 0;
            std::array<Node_Slot*, BRANCH_MAX>      m_branch_slot         = {};
            std::array<Node_Slot*, BRANCH_MAX - 1>  m_condition_in        = {};            
            Node_Slot*                              m_initialization_slot = {nullptr};
            Node_Slot*                              m_iteration_slot      = {nullptr};

            Node_Slot*          branch_out(Branch branch)                       { ASSERT(branch < m_branch_count); return m_branch_slot[branch]; }
            const Node_Slot*    branch_out(Branch branch) const                 { ASSERT(branch < m_branch_count); return m_branch_slot[branch]; }
            size_t              branch_count() const                            { return m_branch_count; }
            const Node*         condition(Branch branch = Branch_TRUE) const    { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]->first_adjacent_node(); }
            Node*               condition(Branch branch = Branch_TRUE)          { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]->first_adjacent_node(); }
            const Node_Slot*    condition_in(Branch branch = Branch_TRUE) const { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]; }
            Node_Slot*          condition_in(Branch branch = Branch_TRUE)       { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]; }    
            Node_Slot*          iteration_slot()                                { return m_iteration_slot; }
            Node_Slot*          initialization_slot()                           { return m_initialization_slot; }
            const Node_Slot*    iteration_slot() const                          { return m_iteration_slot; }
            const Node_Slot*    initialization_slot() const                     { return m_initialization_slot; }
        };

        struct Invokable_State
        {
            Token                                   m_identifier_token = {Token_Type::identifier };
            tools::Function_Descriptor              m_func_type; // not owned
            tools::Inline_Vector8<Node_Slot*>       m_argument_slot;
            tools::Inline_Vector8<Node_Property*>   m_argument_props;
            Node*                                   m_node = nullptr;  // Can't we remove this?!

            Node_Slot*                              get_arg_slot(size_t i) const { return m_argument_slot[i]; }
            tools::Array_View<const Node_Slot*>     get_arg_slots() const { return m_argument_slot; }
            const tools::Function_Descriptor*       get_func_type()const { return &m_func_type; }
            const Token&                            get_identifier_token() const { return m_identifier_token; }
            void                                    set_identifier_token(const Token& tok) { m_identifier_token = tok; }
            Node_Slot*                              lvalue_in() const { return m_argument_slot[0]; }
            Node_Slot*                              rvalue_in() const { return m_argument_slot[1]; }
        };

        struct Variable_State
        { 
            Token         type_token        = {Token_Type::keyword_unknown }; // [int] var  =
            Token         operator_token    = {Token_Type::operator_ };       //  int  var [=]
            VariableFlags m_vflags          = VariableFlag_NONE;
            Node_Slot*    decl_out          = nullptr;
            Node_Slot*    ref_out           = nullptr;

            bool                has_flags(VariableFlags flags)const { return (m_vflags & flags) == flags; };
            void                set_flags(VariableFlags flags) { m_vflags |= flags; }
            void                clear_flags(VariableFlags flags = VariableFlag_ALL) { m_vflags &= ~flags; }
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

//===== CONSTRUCTORS/DESTRUCTORS =======================================================================================
        Node();
        ~Node();
        Node(const Node&) = delete;
        Node(Node&&) = delete;
//===== COMMON MEMBERS and internal structures =========================================================================
        static constexpr size_t                 SELF_PROPERTY_INDEX = 0;

        bool                                    is_initialized    = false;
        std::string                             name              = "";
        Token                                   suffix            = Token{};
        Graph*                                  graph             = nullptr;
        Node_Flags                              flags             = Node_Flag_IS_DIRTY;
        Node_Property*                          value             = nullptr; // Short had for prop_at( self_property_index )
        Scope*                                  scope             = nullptr; 
        Scope*                                  internal_scope    = nullptr;       
        std::vector<Node_Property*>             props; // TODO: size-fixed array?
        std::map<std::string, Node_Property*>   props_by_name;
        tools::Component_Bag<Node>              component_bag;
        std::vector<Node_Slot*>                 slots; // TODO: size-fixed array?
        std::unordered_map<const Node_Property*, std::vector<Node_Slot*>> slots_by_prop;// TODO: if we are sure a property has a fixed index, we could use a vector instead
        Adjacent_Nodes_Cache                    adjacent_nodes_cache;

//===== SIGNALS ========================================================================================================

        tools::Simple_Signal                    signal_shutdown; // emit once shutdown() has been called
        tools::Signal<void(const std::string&)> signal_name_change;                

//===== TAGGED-UNION DATA ==============================================================================================

        Node_Type type = Node_Type_NULL;
        union // depends on type
        {            
            Switch_Behavior_State   _switch_behavior_data;
            Invokable_State         _invokable_data;
            Variable_State          _variable_data;
            Variable_Ref_State      _variable_ref_data;
            Literal_State           _literal_data;
        };

        // decl some safe accessors for each tagged-union data

        inline Switch_Behavior_State&       switch_behavior_data()       { ASSERT(has_switch_behavior());    return _switch_behavior_data; }
        inline const Switch_Behavior_State& switch_behavior_data() const { ASSERT(has_switch_behavior());    return _switch_behavior_data; }
        inline Invokable_State&             invokable_data()             { ASSERT(is_invokable());           return _invokable_data; }
        inline const Invokable_State&       invokable_data() const       { ASSERT(is_invokable());           return _invokable_data; }
        inline Variable_State&              variable_data()              { ASSERT(is_variable());            return _variable_data; }
        inline const Variable_State&        variable_data() const        { ASSERT(is_variable());            return _variable_data; }
        inline Variable_Ref_State&          variable_ref_data()          { ASSERT(is_variable_ref());        return _variable_ref_data; }
        inline const Variable_Ref_State&    variable_ref_data() const    { ASSERT(is_variable_ref());        return _variable_ref_data; }
        inline Literal_State&               literal_data()               { ASSERT(is_literal());             return _literal_data; }
        inline const Literal_State&         literal_data() const         { ASSERT(is_literal());             return _literal_data; }

//===== COMMON METHODS =================================================================================================
        void                        set_name(const std::string& _name) { name = _name; signal_name_change.emit(name); }
        bool                        is_expression() const;
        bool                        is_invokable() const { return type == Node_Type_OPERATOR || type == Node_Type_FUNCTION; }
        bool                        is_variable() const { return type == Node_Type_VARIABLE; }
        bool                        is_variable_ref() const { return type == Node_Type_VARIABLE_REF; }
        bool                        is_literal() const { return type == Node_Type_LITERAL; }
        bool                        has_flags(Node_Flags _flags)const { return (flags & _flags) == _flags; };
        void                        set_flags(Node_Flags _flags) { flags |= flags; }
        void                        clear_flags(Node_Flags _flags = Node_Flag_ALL) { flags &= ~_flags; }
        bool                        is_orphan() const { return scope == nullptr; }
        bool                        has_switch_behavior() const;
//===== SLOT RELATED METHODS ===========================================================================================
        Node_Slot*                value_in();
        const Node_Slot*          value_in() const;
        Node_Slot*                value_out();
        const Node_Slot*          value_out() const;
        Node_Slot*                flow_in();
        const Node_Slot*          flow_in() const;
        Node_Slot*                flow_out();
        const Node_Slot*          flow_out() const;
        Node_Slot*                flow_enter();
        const Node_Slot*          flow_enter() const;
        const std::vector<Node*>& inputs() const       { return adjacent_nodes_cache.get(Node_Slot::Flag_INPUT); }
        const std::vector<Node*>& outputs() const      { return adjacent_nodes_cache.get(Node_Slot::Flag_OUTPUT); }
        const std::vector<Node*>& flow_inputs() const  { return adjacent_nodes_cache.get(Node_Slot::Flag_FLOW_IN); }
        const std::vector<Node*>& flow_outputs() const { return adjacent_nodes_cache.get(Node_Slot::Flag_FLOW_OUT); }
        void                      handle_slot_change(Node_Slot::Event, Node_Slot*);        
    };

//===== API ====================================================================================================

    void                    node_init(Node*, Node_Type, const std::string& name);
    void                    node_init_as_invokable(Node*, const tools::Function_Descriptor&, Node_Type = Node_Type_FUNCTION);
    void                    node_init_as_variable(Node*, const tools::Type_Descriptor* type, const char* identifier);
    void                    node_init_as_variable_ref(Node*);
    void                    node_init_as_literal(Node*, const tools::Type_Descriptor* _type);
    void                    node_init_as_root_scope(Node*);
    void                    node_init_as_scope(Node*);
    void                    node_init_as_cond_struct(Node*);
    void                    node_init_as_for_loop(Node*);
    void                    node_init_as_while_loop(Node*);
    void                    node_init_as_empty_instruction(Node*);
    void                    node_init_internal_scope(Node*);
    void                    node_init_branches(Node*, size_t branch_count);
    void                    node_shutdown(Node*);
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

    std::vector<Node*>      node_get_adjacent_nodes(const Node*, Node_Slot::Flags);
    Node*                   node_adjacent_node_at(const Node*, Node_Slot::Flags, u8_t pos);
    bool                    node_could_be_instruction(const Node*);
    bool                    node_is_instruction(const Node*);
    bool                    node_is_unary_operator(const Node*);
    bool                    node_is_binary_operator(const Node*);
    bool                    node_is_conditional(const Node*);
    bool                    node_is_connected_to_codeflow(const Node *node);
    bool                    node_is_output_node_in_expression(const Node* input_node, const Node* output_node);
    bool                    node_is_initialized(const Node* node);
}
