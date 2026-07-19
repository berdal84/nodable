#pragma once

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

    typedef int Create_Node_Type;
    enum Create_Node_Type_ : int
    {
        Create_Node_Type_ROOT,
        Create_Node_Type_BLOCK_CONDITION,
        Create_Node_Type_BLOCK_FOR_LOOP,
        Create_Node_Type_BLOCK_WHILE_LOOP,
        Create_Node_Type_BLOCK_SCOPE,
        Create_Node_Type_VARIABLE_BOOLEAN,
        Create_Node_Type_VARIABLE_DOUBLE,
        Create_Node_Type_VARIABLE_INTEGER,
        Create_Node_Type_VARIABLE_STRING,
        Create_Node_Type_LITERAL_BOOLEAN,
        Create_Node_Type_LITERAL_DOUBLE,
        Create_Node_Type_LITERAL_INTEGER,
        Create_Node_Type_LITERAL_STRING,
        Create_Node_Type_FUNCTION,
    };

	struct Graph
	{
        Graph() = default;
        ~Graph();
        
        std::vector<Node*>                  nodes; // TODO: since Node does not use virtuals, I should replace this by an Arena.
        tools::Component_Bag<Graph>         component_bag;
        tools::Simple_Signal                signal_reset;
        tools::Simple_Broadcast_Signal      signal_change;
        tools::Signal<void(Node*)>          signal_add_node;
        tools::Signal<void(Node*)>          signal_remove_node;
        tools::Simple_Signal                signal_is_complete; // user defined, usually when parser or user is done

        struct Scope_Change {
            Node*  node;
            Scope* old_scope;
            Scope* new_scope;
        };
        tools::Signal<void(Scope_Change)>   signal_change_scope;
    };

    void                    graph_init(Graph*);
    void                    graph_deinit(Graph*);
    void                    graph_clear(Graph*);
    bool                    graph_update(Graph*);
    void                    graph_reset(Graph*); // Delete all nodes, wires, edges and reset scope.
    inline Node*            graph_root(const Graph* graph)  { return graph->nodes.front(); /* we have the guarantee it exists, see graph_init */}
    inline Scope*           graph_root_scope(const Graph* graph) { return graph_root(graph)->internal_scope; };
    inline bool             graph_is_empty(const Graph* graph)   { return scope_is_empty( graph_root_scope(graph) ); };
    void                    graph_connect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags);
    void                    graph_connect_to_variable(Node_Slot* output_slot, Node* variable );
    void                    graph_connect_or_merge(Node_Slot* tail, Node_Slot* head);
    void                    graph_disconnect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_insert(Graph*, Node*, Scope*);
    void                    graph_clean_node(Node*);
    void                    graph_transfer_children(Scope* /* from */, Scope* /* to */);
    void                    graph_change_scope(Node*, Scope* /*desired_scope*/);
    Node*                   graph_create_node(Graph*, Scope* = nullptr);
    Node*                   graph_create_node(Graph*, Create_Node_Type, const tools::Function_Descriptor*, Scope* = nullptr);
    Node*                   graph_create_variable(Graph*, const tools::Type_Descriptor* type, const std::string& name, Scope* scope  = nullptr);
    Node*                   graph_create_variable_ref(Graph*, Scope* = nullptr);
    Node*                   graph_create_variable_decl(Graph*, const tools::Type_Descriptor* _type, const char* _name, Scope* = nullptr);
    Node*                   graph_create_literal(Graph*, const tools::Type_Descriptor *_type, Scope* = nullptr);
    Node*                   graph_create_function(Graph*, const tools::Function_Descriptor&, Scope* = nullptr);
    Node*                   graph_create_operator(Graph*, const tools::Function_Descriptor&, Scope* = nullptr);
    Node*                   graph_create_cond_struct(Graph*, Scope* = nullptr);
    Node*                   graph_create_for_loop(Graph*, Scope* = nullptr);
    Node*                   graph_create_while_loop(Graph*, Scope* = nullptr);
    Node*                   graph_create_empty_instruction(Graph*, Scope* = nullptr);
    Node*                   graph_create_scope(Graph*, Scope* scope = nullptr);
    void                    graph_find_and_destroy(Graph*, Node* node);
    std::vector<Scope *>    graph_collect_scopes(const Graph*);
    std::set<Scope *>       graph_collect_root_scopes(const Graph*);
    void                    graph_flag_node_to_delete(Node*, Graph_Flags = Graph_Flag_NONE);
    bool                    graph_contains(const Graph*, Node*);

    template<typename T>
    Node* graph_create_variable_decl(Graph* graph, const char* name = "var", Scope* scope = nullptr)
    { return graph_create_variable_decl( graph, tools::type::get<T>(), name, scope); }

    template<typename T>
    Node* graph_create_variable(Graph* graph, const char* name = "var", Scope* scope = nullptr)
    { return graph_create_variable( graph, tools::type::get<T>(), name, scope); }

    template<typename T>
    Node* graph_create_literal(Graph* graph, Scope* scope = nullptr)                          
    { return graph_create_literal( graph, tools::type::get<T>(), scope ); }
}