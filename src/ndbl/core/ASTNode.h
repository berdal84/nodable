#pragma once

#include <string>
#include <array>

#include "tools/core/assertions.h"
#include "tools/core/Component.h"

#include "ASTNodeProperty.h"
#include "ASTNodeSlot.h"
#include "ASTNodeType.h"

namespace ndbl
{
    // forward declarations
    class Graph;
    class ASTScope;
    class ASTNode;
    class ASTNodeSlot;
    
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

    typedef int ASTNodeFlags;
    enum ASTNodeFlag_
    {
        ASTNodeFlag_NONE                = 0,
        ASTNodeFlag_IS_DIRTY            = 1 << 0,
        ASTNodeFlag_WAS_IN_A_SCOPE_ONCE = 1 << 1,
        ASTNodeFlag_MUST_BE_DELETED     = 1 << 2,
        ASTNodeFlag_ALL                 = ~ASTNodeFlag_NONE,
        ASTNodeFlag_DEFAULT             = ASTNodeFlag_NONE,
    };

    class ASTNode
	{
    public:
        DECLARE_REFLECT        
        friend class ASTScope;
        friend class Graph;

//===== INTERNAL STRUCTS ===============================================================================================

        struct AdjacentNodesCache
        {
            // Struct to get a list of nodes from given flags, caches the result

            explicit AdjacentNodesCache(const ASTNode* node): _node(node) {}
            const std::vector<ASTNode*>& get(SlotFlags) const;
            void set_dirty() { _cache.clear(); }
        private:
            const ASTNode* _node;
            std::unordered_map<SlotFlags, std::vector<ASTNode*>> _cache;
        };

        struct SwitchBehaviorData
        {
            // Handle any conditionnal structure, for now it only handle 2 branches, but it will be modified to have N branches (like a switch)

            static constexpr size_t                     BRANCH_MAX          = 2;
            ASTToken                                    m_branch_prefix       = {ASTToken_t::ignore}; // e.g. if|for|while
            ASTToken                                    m_branch_suffix       = {ASTToken_t::ignore}; // e.g. else
            size_t                                      m_branch_count        = 0;
            std::array<ASTNodeSlot*, BRANCH_MAX>        m_branch_slot         = {};
            std::array<ASTNodeSlot*, BRANCH_MAX - 1>    m_condition_in        = {};            
            ASTNodeSlot*                                m_initialization_slot = {nullptr};
            ASTNodeSlot*                                m_iteration_slot      = {nullptr};
            ASTNode*                                    m_node              = nullptr;

            ASTNodeSlot*            branch_out(Branch branch)                       { ASSERT(branch < m_branch_count); return m_branch_slot[branch]; }
            const ASTNodeSlot*      branch_out(Branch branch) const                 { ASSERT(branch < m_branch_count); return m_branch_slot[branch]; }
            size_t                  branch_count() const                            { return m_branch_count; }
            const ASTNode*          condition(Branch branch = Branch_TRUE) const    { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]->first_adjacent_node(); }
            ASTNode*                condition(Branch branch = Branch_TRUE)          { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]->first_adjacent_node(); }
            const ASTNodeSlot*      condition_in(Branch branch = Branch_TRUE) const { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]; }
            ASTNodeSlot*            condition_in(Branch branch = Branch_TRUE)       { ASSERT(Branch_FALSE < branch && branch < m_branch_count); return m_condition_in[branch - 1]; }    
            ASTNodeSlot*            iteration_slot()                                { return m_iteration_slot; }
            ASTNodeSlot*            initialization_slot()                           { return m_initialization_slot; }
            const ASTNodeSlot*      iteration_slot() const                          { return m_iteration_slot; }
            const ASTNodeSlot*      initialization_slot() const                     { return m_initialization_slot; }
        };

        struct InvokableData
        {
            ASTToken                                m_identifier_token = {ASTToken_t::identifier };
            tools::FunctionDescriptor               m_func_type; // not owned
            tools::InlineVector8<ASTNodeSlot*>      m_argument_slot;
            tools::InlineVector8<ASTNodeProperty*>  m_argument_props;
            ASTNode*                                m_node = nullptr;  // Can't we remove this?!

            InvokableData(ASTNode* node); // Can't we remove this?!

            void                                    init(const tools::FunctionDescriptor& func_type);
            ASTNodeSlot*                            get_arg_slot(size_t i) const { return m_argument_slot[i]; }
            tools::ArrayView<const ASTNodeSlot*>    get_arg_slots() const { return m_argument_slot; }
            const tools::FunctionDescriptor*        get_func_type()const { return &m_func_type; }
            const ASTToken&                         get_identifier_token() const { return m_identifier_token; }
            void                                    set_identifier_token(const ASTToken& tok) { m_identifier_token = tok; }
            ASTNodeSlot*                            lvalue_in() const { return m_argument_slot[0]; }
            ASTNodeSlot*                            rvalue_in() const { return m_argument_slot[1]; }
        };

        struct VariableData
        { 
        private:
            ASTToken        m_type_token            = {ASTToken_t::keyword_unknown }; // [int] var  =
            ASTToken        m_operator_token        = {ASTToken_t::operator_ };       //  int  var [=]
            VariableFlags   m_vflags                = VariableFlag_NONE;
            ASTNodeSlot*    m_as_declaration_slot   = nullptr;
            ASTNodeSlot*    m_as_reference_slot     = nullptr;
            ASTNode*        m_owner_node            = nullptr; // Can't we remove this?!

        public:
            VariableData(ASTNode* node); // Can't we remove this?!
            void                            init(const tools::TypeDescriptor* type, const char* identifier);
            bool                            has_flags(VariableFlags flags)const { return (m_vflags & flags) == flags; };
            void                            set_flags(VariableFlags flags) { m_vflags |= flags; }
            void                            clear_flags(VariableFlags flags = VariableFlag_ALL) { m_vflags &= ~flags; }
            const tools::TypeDescriptor*    get_type() const { return m_owner_node->value()->get_type(); }
            const ASTToken&                 get_type_token() const { return m_type_token; }
            std::string                     get_identifier() const { return get_identifier_token().word_to_string(); }
            const ASTToken&                 get_identifier_token() const { return m_owner_node->value()->token(); }
            ASTToken&                       get_identifier_token() { return m_owner_node->value()->token(); }
            const ASTToken&                 get_operator_token() const { return m_operator_token; }
            void                            set_type_token(const ASTToken& tok) { m_type_token = tok; }
            void                            set_identifier_token(const ASTToken& tok) { m_owner_node->value()->set_token(tok); }
            void                            set_operator_token(const ASTToken& tok) { m_operator_token = tok; }

            // Aliases

            ASTNodeSlot*                    decl_out() { return m_as_declaration_slot; }
            const ASTNodeSlot*              decl_out() const { return m_as_declaration_slot; }
            ASTNodeSlot*                    ref_out() { return m_as_reference_slot; }
            const ASTNodeSlot*              ref_out() const { return m_as_reference_slot; }
        };

        struct VariableRefData
        {
            VariableRefData(ASTNode* node); // Can't we remove this?!
            VariableRefData() = default;
            ~VariableRefData();

            void            init();
            void            set_variable(ASTNode* variable);
            void            clear_variable();
            const ASTToken& get_identifier_token() const;

        private:
            void            handle_name_change(const std::string& name);

            ASTNode*        m_owner_node    = nullptr; // Can't we remove this?!
            ASTNode*        m_variable_node = nullptr;
        };

        struct LiteralData
        {
            LiteralData(ASTNode* node);  // Can't we remove this?!
            void init(const tools::TypeDescriptor* _type);

            ASTToken                        token = {ASTToken_t::literal_any};
            const tools::TypeDescriptor*    m_type = nullptr;
            ASTNode*                        m_node = nullptr;  // Can't we remove this?!
        };

//===== CONSTRUCTORS/DESTRUCTORS =======================================================================================
    public:
        ASTNode();
        ASTNode(ASTNodeType type);
        ~ASTNode();
        ASTNode(const ASTNode&) = delete;
        ASTNode(ASTNode&&) = delete;
//===== COMMON MEMBERS and internal structures =========================================================================
    private:

        static constexpr size_t                 SELF_PROPERTY_INDEX = 0;

        bool                                    m_is_initialized    = false;
        std::string                             m_name              = {};
        ASTToken                                m_suffix            = ASTToken{};
        Graph*                                  m_graph             = nullptr;
        ASTNodeFlags                            m_flags             = ASTNodeFlag_IS_DIRTY;
        ASTNodeProperty*                        m_value             = nullptr; // Short had for prop_at( self_property_index )
        ASTScope*                               m_parent_scope      = nullptr; 
        ASTScope*                               m_internal_scope    = nullptr;       
        AdjacentNodesCache                      m_adjacent_nodes_cache;
        std::vector<ASTNodeProperty*>           m_properties; // TODO: size-fixed array?
        std::map<std::string, ASTNodeProperty*> m_properties_by_name;
        tools::ComponentBag<ASTNode>            m_component_collection;
        std::vector<ASTNodeSlot*>               m_slots; // TODO: size-fixed array?
        std::unordered_map<const ASTNodeProperty*, std::vector<ASTNodeSlot*>>m_slots_by_property;// TODO: use multimap?
//===== SIGNALS ========================================================================================================
        tools::SimpleSignal                     signal_shutdown; // emit once shutdown() has been called
        tools::Signal<void(const std::string&)> signal_name_change;                
//===== TAGGED-UNION DATA ==============================================================================================
    public:

        ASTNodeType m_type = ASTNodeType_NULL;

        union // depends on m_type
        {            
            SwitchBehaviorData  m_switch_behavior_data;
            InvokableData       m_invokable_data;
            VariableData        m_variable_data;
            VariableRefData     m_variable_ref_data;
            LiteralData         m_literal_data;
        };

    private:         
        void construct_union_data();
        void destroy_union_data();

    public: // decl some safe accessors for each tagged-union data

        void                                switch_behavior_create_branches(size_t branch_count);
        inline SwitchBehaviorData&          switch_behavior_data()       { ASSERT(has_switch_behavior());    return m_switch_behavior_data; }
        inline const SwitchBehaviorData&    switch_behavior_data() const { ASSERT(has_switch_behavior());    return m_switch_behavior_data; }
        inline InvokableData&               invokable_data()             { ASSERT(is_invokable());           return m_invokable_data; }
        inline const InvokableData&         invokable_data() const       { ASSERT(is_invokable());           return m_invokable_data; }
        inline VariableData&                variable_data()              { ASSERT(is_variable());            return m_variable_data; }
        inline const VariableData&          variable_data() const        { ASSERT(is_variable());            return m_variable_data; }
        inline VariableRefData&             variable_ref_data()          { ASSERT(is_variable_ref());        return m_variable_ref_data; }
        inline const VariableRefData&       variable_ref_data() const    { ASSERT(is_variable_ref());        return m_variable_ref_data; }
        inline LiteralData&                 literal_data()               { ASSERT(is_literal());             return m_literal_data; }
        inline const LiteralData&           literal_data() const         { ASSERT(is_literal());             return m_literal_data; }

//===== COMMON METHODS =================================================================================================
    public:
        void                        init(const std::string& name);
        void                        shutdown();
        const std::string&          name() const { return m_name; }
        void                        set_name(const std::string& name) { m_name = name; signal_name_change.emit(name); }
        bool                        update();
        ASTNodeType                 type() const { return m_type; }
        bool                        is_initialized() const { return m_is_initialized; }
        bool                        is_expression() const;
        bool                        is_invokable() const { return m_type == ASTNodeType_OPERATOR || m_type == ASTNodeType_FUNCTION; }
        bool                        is_variable() const { return m_type == ASTNodeType_VARIABLE; }
        bool                        is_variable_ref() const { return m_type == ASTNodeType_VARIABLE_REF; }
        bool                        is_literal() const { return m_type == ASTNodeType_LITERAL; }
        bool                        has_flags(ASTNodeFlags flags)const { return (m_flags & flags) == flags; };
        void                        set_flags(ASTNodeFlags flags) { m_flags |= flags; }
        void                        clear_flags(ASTNodeFlags flags = ASTNodeFlag_ALL) { m_flags &= ~flags; }
        Graph*                      graph() { return m_graph; }
        const Graph*                graph() const { return m_graph; }
        ASTToken&                   suffix() { return m_suffix; };
        const ASTToken&             suffix() const { return m_suffix; };
        void                        set_suffix(const ASTToken& token);
        bool                        is_orphan() const { return m_parent_scope == nullptr; }
        ASTScope*                   scope() const { return m_parent_scope; };
        bool                        has_scope() const { return m_parent_scope != nullptr; }
        void                        init_internal_scope();
        bool                        has_internal_scope() const  { return m_internal_scope != nullptr; }
        bool                        has_switch_behavior() const;
        ASTScope*                   internal_scope() const      { return m_internal_scope; }
    protected:
        void                        reset_scope(ASTScope*);
//===== SLOT RELATED METHODS ===========================================================================================
    public:
        ASTNodeSlot*                value_in();
        const ASTNodeSlot*          value_in() const;
        ASTNodeSlot*                value_out();
        const ASTNodeSlot*          value_out() const;
        ASTNodeSlot*                flow_in();
        const ASTNodeSlot*          flow_in() const;
        ASTNodeSlot*                flow_out();
        const ASTNodeSlot*          flow_out() const;
        ASTNodeSlot*                flow_enter();
        const ASTNodeSlot*          flow_enter() const;
        ASTNodeSlot*                add_slot(ASTNodeProperty *, SlotFlags, size_t limit_capacity = 0, size_t _position = 0);
        size_t                      adjacent_slot_count(SlotFlags flags)const { return filter_adjacent_slots(flags).size(); }
        ASTNodeSlot*                slot_at(size_t pos) { return m_slots.at(pos); }
        const ASTNodeSlot*          slot_at(size_t pos) const { return m_slots.at(pos); }
        std::vector<ASTNodeSlot*>   filter_slots(SlotFlags) const;
        std::vector<ASTNodeSlot*>   filter_slots(const std::function<bool(const ASTNodeSlot*)>& predicate) const;
        std::vector<ASTNodeSlot*>   filter_adjacent_slots(SlotFlags) const;
        ASTNodeSlot*                find_slot(SlotFlags flags) { return find_slot_by_property(m_value, flags ); }// implicitly DEFAULT_PROPERTY's slot
        const ASTNodeSlot*          find_slot(SlotFlags flags) const { return find_slot_by_property(m_value, flags ); }// implicitly DEFAULT_PROPERTY's slot
        ASTNodeSlot*                find_slot_at(SlotFlags flags, size_t pos ) { return const_cast<ASTNodeSlot*>( const_cast<const ASTNode*>(this)->find_slot_at(flags, pos)); } // implicitly DEFAULT_PROPERTY's slot
        const ASTNodeSlot*          find_slot_at(SlotFlags, size_t _position ) const; // implicitly DEFAULT_PROPERTY's slot
        ASTNodeSlot*                find_slot_by_property_name(const char* name, SlotFlags flags) { return const_cast<ASTNodeSlot*>( const_cast<const ASTNode*>(this)->find_slot_by_property_name(name, flags) ); };
        const ASTNodeSlot*          find_slot_by_property_name(const char* name, SlotFlags ) const;
        ASTNodeSlot*                find_slot_by_property_type(SlotFlags _way, const tools::TypeDescriptor *_type) const;
        ASTNodeSlot*                find_slot_by_property(const ASTNodeProperty* prop, SlotFlags flags ) { return const_cast<ASTNodeSlot*>( const_cast<const ASTNode*>( this )->find_slot_by_property(prop, flags ) ); }
        const ASTNodeSlot*          find_slot_by_property(const ASTNodeProperty*, SlotFlags ) const;
        ASTNodeSlot*                find_adjacent_at(SlotFlags, size_t _index ) const;
        size_t                      slot_count(SlotFlags flags) const { return filter_slots( flags ).size(); }
        std::vector<ASTNodeSlot*>&  slots() { return m_slots; }
        const std::vector<ASTNodeSlot*>& slots() const { return m_slots; }
        bool                        has_flow_adjacent() const;
        const std::vector<ASTNode*>& inputs() const       { return m_adjacent_nodes_cache.get(SlotFlag_INPUT); }
        const std::vector<ASTNode*>& outputs() const      { return m_adjacent_nodes_cache.get(SlotFlag_OUTPUT); }
        const std::vector<ASTNode*>& flow_inputs() const  { return m_adjacent_nodes_cache.get(SlotFlag_FLOW_IN); }
        const std::vector<ASTNode*>& flow_outputs() const { return m_adjacent_nodes_cache.get(SlotFlag_FLOW_OUT); }
    protected:
        void                        _handle_slot_change(ASTNodeSlot::Event event, ASTNodeSlot *slot);
//===== PROPERTY RELATED METHODS =======================================================================================
    public:
        const ASTNodeProperty*  value() const { return m_value; }
        ASTNodeProperty*        value() { return m_value; }
        std::vector<ASTNodeProperty*>& props() { return m_properties; }
        const std::vector<ASTNodeProperty*>& props() const { return m_properties; }
        ASTNodeProperty*        add_prop(const tools::TypeDescriptor*, const char* name, PropertyFlags = PropertyFlag_NONE);
        ASTNodeProperty*        get_prop(const char* _name) { return find_prop_by_name( _name ); }
        const ASTNodeProperty*  get_prop(const char* _name) const { return find_prop_by_name( _name ); }
        const tools::FunctionDescriptor* get_connected_function_type(const char *property_name) const; //
        bool                    has_input_connected( const ASTNodeProperty*) const;
        bool                    has_prop(const char*) const;
        ASTNodeProperty*        prop_at(size_t pos) { return m_properties.at(pos); }
        const ASTNodeProperty*  prop_at(size_t pos ) const { return m_properties.at(pos); }
        ASTNodeProperty*        find_prop_by_name(const char* name) { return const_cast<ASTNodeProperty*>( const_cast<const ASTNode*>(this)->find_prop_by_name(name) );}
        const ASTNodeProperty*  find_prop_by_name(const char* name) const;
        ASTNodeProperty*        find_first_prop(PropertyFlags flags, const tools::TypeDescriptor* type ) { return const_cast<ASTNodeProperty*>( const_cast<const ASTNode*>(this)->find_first_prop(flags, type) );}
        const ASTNodeProperty*  find_first_prop(PropertyFlags, const tools::TypeDescriptor* ) const;
        ASTNodeProperty*        get_this_property() { return m_properties.at(SELF_PROPERTY_INDEX); }
        const ASTNodeProperty*  get_this_property() const { return m_properties.at(SELF_PROPERTY_INDEX); }
        template<typename T>
        ASTNodeProperty*        add_prop(const char* name, PropertyFlags flags = PropertyFlag_NONE ) { return add_prop(tools::type::get<T>(), name, flags); }
//===== COMPONENT RELATED METHODS ======================================================================================
    public:
        template<class T> T*                component() const  { return m_component_collection.get<T>(); }
        tools::ComponentBag<ASTNode>*       components()       { return &m_component_collection; }
        const tools::ComponentBag<ASTNode>* components() const { return &m_component_collection; }
    };
}
