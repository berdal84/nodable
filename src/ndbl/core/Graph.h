#pragma once

#include <map>
#include <string>
#include <vector>
#include <set>

#include "tools/core/Component.h" // for Component_Bag<T>
#include "Node.h"
#include "Scope.h"

namespace ndbl
{
    // forward decl
    class Scope;

    typedef int Graph_Flags;
    enum Graph_Flag_
    {
        Graph_Flag_NONE               = 0,
        Graph_Flag_ALLOW_SIDE_EFFECTS = 1 << 0,
    };

    enum Create_Node_Type_
    {
        Create_Node_Type__ROOT,
        Create_Node_Type__BLOCK_CONDITION,
        Create_Node_Type__BLOCK_FOR_LOOP,
        Create_Node_Type__BLOCK_WHILE_LOOP,
        Create_Node_Type__BLOCK_SCOPE,
        Create_Node_Type__VARIABLE_BOOLEAN,
        Create_Node_Type__VARIABLE_DOUBLE,
        Create_Node_Type__VARIABLE_INTEGER,
        Create_Node_Type__VARIABLE_STRING,
        Create_Node_Type__LITERAL_BOOLEAN,
        Create_Node_Type__LITERAL_DOUBLE,
        Create_Node_Type__LITERAL_INTEGER,
        Create_Node_Type__LITERAL_STRING,
        Create_Node_Type__FUNCTION,
    };

    typedef std::vector<Node*> Node_Registry;

    /**
     * @brief To manage a graph (primary_child and edges)
     */
	class Graph
	{
	public:
 		Graph();
		~Graph();
//====== Data =======================================================================================================
    public:
        tools::Simple_Signal            signal_reset;
        tools::Simple_Broadcast_Signal  signal_change;
        tools::Signal<void(Node*)>      signal_add_node;
        tools::Signal<void(Node*)>      signal_remove_node;
        using ScopeChanged = void(Node*, Scope* /* old_scope */, Scope* /* new_scope */ ) ;
        tools::Signal<ScopeChanged>     signal_change_scope;
        tools::Simple_Signal            signal_is_complete; // user defined, usually when parser or user is done
    private:
        Node_Registry                   m_node_registry;
        tools::Component_Bag<Graph>     m_components;
//====== Common Methods ================================================================================================
    public:
        bool                    update();
        void                    reset();  // Delete all nodes, wires, edges and reset scope.
        bool                    is_empty() const { return root_scope()->empty(); };
        Node*                   root_node() const { return m_node_registry.front(); /* we have the guarantee it exists, see constructor */}
        Scope*                  root_scope() const;
        template<class T> T*               component() const  { return m_components.get<T>(); }
        tools::Component_Bag<Graph>*       components()       { return &m_components; }
        const tools::Component_Bag<Graph>* components() const { return &m_components; }
    private:
        void                    _init();
        void                    _clear();
//====== Node(s) Related ===============================================================================================
    public:
        Node*                   create_node() { return create_node( this->root_scope() ); }
        Node*                   create_node(Scope*); // Create a raw node.
        Node*                   create_node(Create_Node_Type_ type, const tools::Function_Descriptor* desc = nullptr) { return create_node(type, desc, this->root_scope()); }
        Node*                   create_node(Create_Node_Type_, const tools::Function_Descriptor*, Scope*);
        Node*                   create_variable(const tools::Type_Descriptor* type, const std::string& name) { return create_variable(type, name, this->root_scope()); }
        Node*                   create_variable(const tools::Type_Descriptor* type, const std::string& name, Scope* scope );
        Node*                   create_variable_ref() { return create_variable_ref( this->root_scope()); }
        Node*                   create_variable_ref(Scope*);
        Node*                   create_variable_decl(const tools::Type_Descriptor* type, const char* name) { return create_variable_decl(type, name, this->root_scope()); }
        Node*                   create_variable_decl(const tools::Type_Descriptor* _type, const char* _name, Scope*);
        Node*                   create_literal(const tools::Type_Descriptor* type) { return create_literal(type, this->root_scope()); }
        Node*                   create_literal(const tools::Type_Descriptor *_type, Scope*);
        Node*                   create_function(const tools::Function_Descriptor& desc) { return create_function(desc, this->root_scope()); }
        Node*                   create_function(const tools::Function_Descriptor&, Scope*);
        Node*                   create_operator(const tools::Function_Descriptor& desc) { return create_operator(desc, this->root_scope()); }
        Node*                   create_operator(const tools::Function_Descriptor&, Scope*);
        Node*                   create_cond_struct() { return create_cond_struct(root_scope()); }
        Node*                   create_cond_struct(Scope*);
        Node*                   create_for_loop() { return create_for_loop(root_scope()); }
        Node*                   create_for_loop(Scope*);
        Node*                   create_while_loop() { return create_while_loop(root_scope()); }
        Node*                   create_while_loop(Scope*);
        Node*                   create_empty_instruction() { return create_empty_instruction(root_scope()); }
        Node*                   create_empty_instruction(Scope*);
        Node*                   create_scope(Scope* scope);
        void                    find_and_destroy(Node* node);
        std::vector<Scope *>    scopes();
        std::set<Scope *>       root_scopes();
        Node_Registry&          nodes() {return m_node_registry;}
        const Node_Registry&    nodes()const {return m_node_registry;}
        void                    flag_node_to_delete(Node* node, Graph_Flags = Graph_Flag_NONE);
        bool                    contains(Node*) const;

        template<typename T> Node* create_variable_decl(const char* name = "var")          { return create_variable_decl( tools::type::get<T>(), name, this->root_scope()); }
        template<typename T> Node* create_variable_decl(const char* name, Scope* scope ){ return create_variable_decl( tools::type::get<T>(), name, scope); }
        template<typename T> Node* create_literal()                 { return create_literal( tools::type::get<T>(), this->root_scope() ); }
        template<typename T> Node* create_literal(Scope* scope ) { return create_literal( tools::type::get<T>(), scope ); }
    private:
        void                    _insert(Node*, Scope*);
        void                    _remove(Node*);
        void                    _clean_node(Node* node);

        void                    _reset_scope(Node* scoped_node);
        void                    _transfer_children(Scope* from, Scope* to);
        void                    _change_scope(Node *node, Scope* desired_scope);
//====== Edge(s) Related ===============================================================================================
    public:
        void                    connect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
        void                    connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags);
        void                    connect_to_variable(Node_Slot* output_slot, Node* variable );
        void                    connect_or_merge(Node_Slot* tail, Node_Slot* head);
        void                    disconnect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    };
}