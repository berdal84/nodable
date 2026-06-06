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

    class Node
	{
    public:
        DECLARE_REFLECT        
        friend class Scope;
        friend class Graph;

//===== INTERNAL STRUCTS ===============================================================================================

        struct Adjacent_Nodes_Cache
        {
            // Struct to get a list of nodes from given flags, caches the result

            explicit Adjacent_Nodes_Cache(const Node* node): _node(node) {}
            const std::vector<Node*>& get(Node_Slot_Flags) const;
            void set_dirty() { _cache.clear(); }
        private:
            const Node* _node;
            std::unordered_map<Node_Slot_Flags, std::vector<Node*>> _cache;
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
            Node*                                   m_node                = nullptr;

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

            Invokable_State(Node* node); // Can't we remove this?!

            void                                    init(const tools::Function_Descriptor& func_type);
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
        private:
            Token         m_type_token            = {Token_Type::keyword_unknown }; // [int] var  =
            Token         m_operator_token        = {Token_Type::operator_ };       //  int  var [=]
            VariableFlags m_vflags                = VariableFlag_NONE;
            Node_Slot*    m_as_declaration_slot   = nullptr;
            Node_Slot*    m_as_reference_slot     = nullptr;
            Node*         m_owner_node            = nullptr; // Can't we remove this?!

        public:
            Variable_State(Node* node); // Can't we remove this?!
            void                            init(const tools::Type_Descriptor* type, const char* identifier);
            bool                            has_flags(VariableFlags flags)const { return (m_vflags & flags) == flags; };
            void                            set_flags(VariableFlags flags) { m_vflags |= flags; }
            void                            clear_flags(VariableFlags flags = VariableFlag_ALL) { m_vflags &= ~flags; }
            const tools::Type_Descriptor*   get_type() const { return m_owner_node->value()->get_type(); }
            const Token&                    get_type_token() const { return m_type_token; }
            std::string                     get_identifier() const { return get_identifier_token().word_to_string(); }
            const Token&                    get_identifier_token() const { return m_owner_node->value()->token(); }
            Token&                          get_identifier_token() { return m_owner_node->value()->token(); }
            const Token&                    get_operator_token() const { return m_operator_token; }
            void                            set_type_token(const Token& tok) { m_type_token = tok; }
            void                            set_identifier_token(const Token& tok) { m_owner_node->value()->set_token(tok); }
            void                            set_operator_token(const Token& tok) { m_operator_token = tok; }

            // Aliases

            Node_Slot*                    decl_out() { return m_as_declaration_slot; }
            const Node_Slot*              decl_out() const { return m_as_declaration_slot; }
            Node_Slot*                    ref_out() { return m_as_reference_slot; }
            const Node_Slot*              ref_out() const { return m_as_reference_slot; }
        };

        struct Variable_Ref_State
        {
            Variable_Ref_State(Node* node); // Can't we remove this?!
            Variable_Ref_State() = default;
            ~Variable_Ref_State();

            void            init();
            void            set_variable(Node* variable);
            void            clear_variable();
            const Token& get_identifier_token() const;

        private:
            void            handle_name_change(const std::string& name);

            Node*        m_owner_node    = nullptr; // Can't we remove this?!
            Node*        m_variable_node = nullptr;
        };

        struct Literal_State
        {
            Literal_State(Node* node);  // Can't we remove this?!
            void init(const tools::Type_Descriptor* _type);

            Token                           token = {Token_Type::literal_any};
            const tools::Type_Descriptor*   type  = nullptr;
            Node*                           node  = nullptr;  // Can't we remove this?!
        };

//===== CONSTRUCTORS/DESTRUCTORS =======================================================================================
    public:
        Node();
        Node(Node_Type type);
        ~Node();
        Node(const Node&) = delete;
        Node(Node&&) = delete;
//===== COMMON MEMBERS and internal structures =========================================================================
    private:

        static constexpr size_t                 SELF_PROPERTY_INDEX = 0;

        bool                                    m_is_initialized    = false;
        std::string                             m_name              = {};
        Token                                   m_suffix            = Token{};
        Graph*                                  m_graph             = nullptr;
        Node_Flags                              m_flags             = Node_Flag_IS_DIRTY;
        Node_Property*                          m_value             = nullptr; // Short had for prop_at( self_property_index )
        Scope*                                  m_parent_scope      = nullptr; 
        Scope*                                  m_internal_scope    = nullptr;       
        Adjacent_Nodes_Cache                    m_adjacent_nodes_cache;
        std::vector<Node_Property*>             m_properties; // TODO: size-fixed array?
        std::map<std::string, Node_Property*>   m_properties_by_name;
        tools::Component_Bag<Node>              m_component_collection;
        std::vector<Node_Slot*>                 m_slots; // TODO: size-fixed array?
        std::unordered_map<const Node_Property*, std::vector<Node_Slot*>>m_slots_by_property;// TODO: use multimap?
//===== SIGNALS ========================================================================================================
        tools::Simple_Signal                     signal_shutdown; // emit once shutdown() has been called
        tools::Signal<void(const std::string&)> signal_name_change;                
//===== TAGGED-UNION DATA ==============================================================================================
    public:

        Node_Type m_type = Node_Type_NULL;

        union // depends on m_type
        {            
            Switch_Behavior_State   m_switch_behavior_data;
            Invokable_State         m_invokable_data;
            Variable_State          m_variable_data;
            Variable_Ref_State      m_variable_ref_data;
            Literal_State           m_literal_data;
        };

    private:         
        void construct_union_data();
        void destroy_union_data();

    public: // decl some safe accessors for each tagged-union data

        void                                switch_behavior_create_branches(size_t branch_count);
        inline Switch_Behavior_State&       switch_behavior_data()       { ASSERT(has_switch_behavior());    return m_switch_behavior_data; }
        inline const Switch_Behavior_State& switch_behavior_data() const { ASSERT(has_switch_behavior());    return m_switch_behavior_data; }
        inline Invokable_State&             invokable_data()             { ASSERT(is_invokable());           return m_invokable_data; }
        inline const Invokable_State&       invokable_data() const       { ASSERT(is_invokable());           return m_invokable_data; }
        inline Variable_State&              variable_data()              { ASSERT(is_variable());            return m_variable_data; }
        inline const Variable_State&        variable_data() const        { ASSERT(is_variable());            return m_variable_data; }
        inline Variable_Ref_State&          variable_ref_data()          { ASSERT(is_variable_ref());        return m_variable_ref_data; }
        inline const Variable_Ref_State&    variable_ref_data() const    { ASSERT(is_variable_ref());        return m_variable_ref_data; }
        inline Literal_State&               literal_data()               { ASSERT(is_literal());             return m_literal_data; }
        inline const Literal_State&         literal_data() const         { ASSERT(is_literal());             return m_literal_data; }

//===== COMMON METHODS =================================================================================================
    public:
        void                        init(const std::string& name);
        void                        shutdown();
        const std::string&          name() const { return m_name; }
        void                        set_name(const std::string& name) { m_name = name; signal_name_change.emit(name); }
        bool                        update();
        Node_Type                   type() const { return m_type; }
        bool                        is_initialized() const { return m_is_initialized; }
        bool                        is_expression() const;
        bool                        is_invokable() const { return m_type == Node_Type_OPERATOR || m_type == Node_Type_FUNCTION; }
        bool                        is_variable() const { return m_type == Node_Type_VARIABLE; }
        bool                        is_variable_ref() const { return m_type == Node_Type_VARIABLE_REF; }
        bool                        is_literal() const { return m_type == Node_Type_LITERAL; }
        bool                        has_flags(Node_Flags flags)const { return (m_flags & flags) == flags; };
        void                        set_flags(Node_Flags flags) { m_flags |= flags; }
        void                        clear_flags(Node_Flags flags = Node_Flag_ALL) { m_flags &= ~flags; }
        Graph*                      graph() { return m_graph; }
        const Graph*                graph() const { return m_graph; }
        Token&                      suffix() { return m_suffix; };
        const Token&                suffix() const { return m_suffix; };
        void                        set_suffix(const Token& token);
        bool                        is_orphan() const { return m_parent_scope == nullptr; }
        Scope*                      scope() const { return m_parent_scope; };
        bool                        has_scope() const { return m_parent_scope != nullptr; }
        void                        init_internal_scope();
        bool                        has_internal_scope() const  { return m_internal_scope != nullptr; }
        bool                        has_switch_behavior() const;
        Scope*                      internal_scope() const      { return m_internal_scope; }
    protected:
        void                        reset_scope(Scope*);
//===== SLOT RELATED METHODS ===========================================================================================
    public:
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
        Node_Slot*                add_slot(Node_Property *, Node_Slot_Flags, size_t limit_capacity = 0, size_t _position = 0);
        size_t                    adjacent_slot_count(Node_Slot_Flags flags)const { return filter_adjacent_slots(flags).size(); }
        Node_Slot*                slot_at(size_t pos) { return m_slots.at(pos); }
        const Node_Slot*          slot_at(size_t pos) const { return m_slots.at(pos); }
        std::vector<Node_Slot*>   filter_slots(Node_Slot_Flags) const;
        std::vector<Node_Slot*>   filter_slots(const std::function<bool(const Node_Slot*)>& predicate) const;
        std::vector<Node_Slot*>   filter_adjacent_slots(Node_Slot_Flags) const;
        Node_Slot*                find_slot(Node_Slot_Flags flags) { return find_slot_by_property(m_value, flags ); }// implicitly DEFAULT_PROPERTY's slot
        const Node_Slot*          find_slot(Node_Slot_Flags flags) const { return find_slot_by_property(m_value, flags ); }// implicitly DEFAULT_PROPERTY's slot
        Node_Slot*                find_slot_at(Node_Slot_Flags flags, size_t pos ) { return const_cast<Node_Slot*>( const_cast<const Node*>(this)->find_slot_at(flags, pos)); } // implicitly DEFAULT_PROPERTY's slot
        const Node_Slot*          find_slot_at(Node_Slot_Flags, size_t _position ) const; // implicitly DEFAULT_PROPERTY's slot
        Node_Slot*                find_slot_by_property_name(const char* name, Node_Slot_Flags flags) { return const_cast<Node_Slot*>( const_cast<const Node*>(this)->find_slot_by_property_name(name, flags) ); };
        const Node_Slot*          find_slot_by_property_name(const char* name, Node_Slot_Flags ) const;
        Node_Slot*                find_slot_by_property_type(Node_Slot_Flags _way, const tools::Type_Descriptor *_type) const;
        Node_Slot*                find_slot_by_property(const Node_Property* prop, Node_Slot_Flags flags ) { return const_cast<Node_Slot*>( const_cast<const Node*>( this )->find_slot_by_property(prop, flags ) ); }
        const Node_Slot*          find_slot_by_property(const Node_Property*, Node_Slot_Flags ) const;
        Node_Slot*                find_adjacent_at(Node_Slot_Flags, size_t _index ) const;
        size_t                      slot_count(Node_Slot_Flags flags) const { return filter_slots( flags ).size(); }
        std::vector<Node_Slot*>&  slots() { return m_slots; }
        const std::vector<Node_Slot*>& slots() const { return m_slots; }
        bool                        has_flow_adjacent() const;
        const std::vector<Node*>& inputs() const       { return m_adjacent_nodes_cache.get(Node_Slot_Flag_INPUT); }
        const std::vector<Node*>& outputs() const      { return m_adjacent_nodes_cache.get(Node_Slot_Flag_OUTPUT); }
        const std::vector<Node*>& flow_inputs() const  { return m_adjacent_nodes_cache.get(Node_Slot_Flag_FLOW_IN); }
        const std::vector<Node*>& flow_outputs() const { return m_adjacent_nodes_cache.get(Node_Slot_Flag_FLOW_OUT); }
    protected:
        void                        _handle_slot_change(Node_Slot::Event event, Node_Slot *slot);
//===== PROPERTY RELATED METHODS =======================================================================================
    public:
        const Node_Property*    value() const { return m_value; }
        Node_Property*          value() { return m_value; }
        std::vector<Node_Property*>& props() { return m_properties; }
        const std::vector<Node_Property*>& props() const { return m_properties; }
        Node_Property*          add_prop(const tools::Type_Descriptor*, const char* name, Node_Property_Flags = Node_Property_Flag_NONE);
        Node_Property*          get_prop(const char* _name) { return find_prop_by_name( _name ); }
        const Node_Property*    get_prop(const char* _name) const { return find_prop_by_name( _name ); }
        const tools::Function_Descriptor* get_connected_function_type(const char *property_name) const; //
        bool                    has_input_connected( const Node_Property*) const;
        bool                    has_prop(const char*) const;
        Node_Property*          prop_at(size_t pos) { return m_properties.at(pos); }
        const Node_Property*    prop_at(size_t pos ) const { return m_properties.at(pos); }
        Node_Property*          find_prop_by_name(const char* name) { return const_cast<Node_Property*>( const_cast<const Node*>(this)->find_prop_by_name(name) );}
        const Node_Property*    find_prop_by_name(const char* name) const;
        Node_Property*          find_first_prop(Node_Property_Flags flags, const tools::Type_Descriptor* type ) { return const_cast<Node_Property*>( const_cast<const Node*>(this)->find_first_prop(flags, type) );}
        const Node_Property*    find_first_prop(Node_Property_Flags, const tools::Type_Descriptor* ) const;
        Node_Property*          get_this_property() { return m_properties.at(SELF_PROPERTY_INDEX); }
        const Node_Property*    get_this_property() const { return m_properties.at(SELF_PROPERTY_INDEX); }
        template<typename T>    
        Node_Property*          add_prop(const char* name, Node_Property_Flags flags = Node_Property_Flag_NONE ) { return add_prop(tools::type::get<T>(), name, flags); }
//===== COMPONENT RELATED METHODS ======================================================================================
    public:
        template<class T> T*                component() const  { return m_component_collection.get<T>(); }
        tools::Component_Bag<Node>*         components()       { return &m_component_collection; }
        const tools::Component_Bag<Node>*   components() const { return &m_component_collection; }
    };

//===== UTILITIES ====================================================================================================

    // TODO:
    //  - convert Node to a struct with constr and destr
    //  - move methods here as node_xxxx(Node*) or node_xxx(const Node*)
    //

    // Factory

    Node*                   node_create_root_scope();
    Node*                   node_create_scope();
    Node*                   node_create_variable(const tools::Type_Descriptor*, const std::string& name);
    Node*                   node_create_variable_ref();
    Node*                   node_create_literal(const tools::Type_Descriptor*);
    Node*                   node_create_function(const tools::Function_Descriptor&, Node_Type = Node_Type_FUNCTION);
    Node*                   node_create_cond_struct();
    Node*                   node_create_for_loop();
    Node*                   node_create_while_loop();
    Node*                   node_create_node();
    Node*                   node_create_empty_instruction();

    // Misc.

    std::vector<Node*>      node_get_adjacent_nodes(const Node*, Node_Slot_Flags);
    Node*                   node_adjacent_node_at(const Node*, Node_Slot_Flags, u8_t pos);
    bool                    node_is_instruction(const Node*);
    bool                    node_can_be_instruction(const Node*);
    bool                    node_is_unary_operator(const Node*);
    bool                    node_is_binary_operator(const Node*);
    bool                    node_is_conditional(const Node*);
    bool                    node_is_connected_to_codeflow(const Node *node);
    bool                    node_is_output_node_in_expression(const Node* input_node, const Node* output_node);
    bool                    node_is_initialized(const Node* node);
}
