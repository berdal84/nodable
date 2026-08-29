#pragma once

#include <cstddef>
#include <unordered_map>
#include <vector>
#include <set>

#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include "bdc/String_Hash.hpp"
#include "bdc/Types.hpp"

#include "core/reflection/Type_Descriptor.h"
#include "ndbl/gui/Event.h"
#include "Node.h"
#include "Scope.h"

namespace ndbl
{
    // forward decl
    struct Scope;
    struct Graph_View;

    typedef int Graph_Flags;
    enum Graph_Flag_
    {
        Graph_Flag_NONE               = 0,
        Graph_Flag_ALLOW_SIDE_EFFECTS = 1 << 0,
    };

	struct Graph
	{       
        #define NODE_MAX_COUNT 4096

        bdc::Hash_Map<bdc::String, size_t>      node_index_by_name;
        bdc::Inlined_Array<Node, NODE_MAX_COUNT> nodes;

        Graph_View*                         view;
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

        Graph() = default;
        Graph(const Graph& other) { *this = other; }
        Graph& operator=(const Graph& other) { if (this != &other) memcpy((void*)this, (void*)&other, sizeof(Graph)); return *this; }
    };

    void                    graph_init(Graph*);
    void                    graph_deinit(Graph*);
    void                    graph_clear(Graph*);
    bool                    graph_update(Graph*);
    void                    graph_reset(Graph*); // Delete all nodes, wires, edges and reset scope.
    inline Node*            graph_root(Graph* graph) { return &graph->nodes[0]; /* we have the guarantee it exists, see graph_init */}
    inline const Node*      graph_root(const Graph* graph) { return &graph->nodes[0]; }
    inline Scope*           graph_root_scope(const Graph* graph) { return graph->nodes[0].internal_scope; };
    inline bool             graph_is_empty(const Graph* graph)   { return scope_is_empty( graph_root_scope(graph) ); };
    Node*                   graph_find_node(Graph* graph, const bdc::String_Hash& id);
    void                    graph_find_and_destroy_node(Graph*, Node* node);
    void                    graph_connect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags);
    void                    graph_connect_to_variable(Node_Slot* output_slot, Node* variable );
    void                    graph_connect_or_merge(Node_Slot* tail, Node_Slot* head);
    void                    graph_disconnect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_clean_node(Node*);
    void                    graph_transfer_children(Scope* /* from */, Scope* /* to */);
    void                    graph_change_scope(Node*, Scope* /*desired_scope*/);
    Node*                   graph_create_node(Graph*, Scope* = nullptr);
    Node*                   graph_create_variable(Graph*, const tools::Type_Descriptor*, const bdc::String& name, Scope* scope  = nullptr);
    Node*                   graph_create_variable_ref(Graph*, Scope* = nullptr);
    Node*                   graph_create_variable_decl(Graph*, const tools::Type_Descriptor*, const bdc::String _name, Scope* = nullptr);
    Node*                   graph_create_literal(Graph*, const tools::Type_Descriptor*, Scope* = nullptr);
    Node*                   graph_create_function(Graph*, const tools::Function_Descriptor*, Scope* = nullptr);
    Node*                   graph_create_operator(Graph*, const tools::Function_Descriptor*, Scope* = nullptr);
    Node*                   graph_create_cond_struct(Graph*, Scope* = nullptr);
    Node*                   graph_create_for_loop(Graph*, Scope* = nullptr);
    Node*                   graph_create_while_loop(Graph*, Scope* = nullptr);
    Node*                   graph_create_empty_instruction(Graph*, Scope* = nullptr);
    Node*                   graph_create_scope(Graph*, Scope* scope = nullptr);
    Node*                   graph_create_return(Graph*, const tools::Type_Descriptor*, Scope* = nullptr);
    Node*                   graph_create_node(Graph*, const Node_State*, Scope* = nullptr);
    std::vector<Scope *>    graph_collect_scopes(const Graph*);
    std::set<Scope *>       graph_collect_root_scopes(const Graph*);
    void                    graph_flag_node_to_delete(Node*, Graph_Flags = Graph_Flag_NONE);
    bool                    graph_contains(const Graph*, Node*);
    Node*                   graph_get_latest_created_node(Graph*);
    const Node*             graph_get_latest_created_node(const Graph*);

    template<typename T>
    Node* graph_create_variable_decl(Graph* graph, const bdc::String name = "var", Scope* scope = nullptr)
    { return graph_create_variable_decl( graph, tools::type::get<T>(), name, scope); }

    template<typename T>
    Node* graph_create_variable(Graph* graph, const bdc::String name = "var", Scope* scope = nullptr)
    { return graph_create_variable( graph, tools::type::get<T>(), name, scope); }

    template<typename T>
    Node* graph_create_literal(Graph* graph, Scope* scope = nullptr)                          
    { return graph_create_literal( graph, tools::type::get<T>(), scope ); }
}