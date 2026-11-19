#pragma once

#include "bdc/String.hpp"
#include "bdc/Array.hpp"
#include "bdc/String_Hash.hpp"
#include "bdc/Types.hpp"
#include "reflection/Type_Descriptor.h"
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
        bdc::Hash_Map<Node_ID, Node*>       node_by_id;
        Node_ID                             root_node_id        = {};
        Graph_View*                         view                = {};
        Simple_Signal                       signal_reset        = {};
        Simple_Broadcast_Signal             signal_change       = {};
        Signal<void(Node*)>                 signal_add_node     = {};
        Signal<void(Node*)>                 signal_remove_node  = {};
        Simple_Signal                       signal_is_complete  = {}; // user defined, usually when parser or user is done

        struct Scope_Change {
            Node*  node;
            Scope* old_scope;
            Scope* new_scope;
        };
        Signal<void(Scope_Change)>   signal_change_scope;
    };

    void                    graph_init(Graph*);
    void                    graph_deinit(Graph*);
    void                    graph_add(Graph*, Node*, Scope*);
    void                    graph_remove(Graph*, Node*);
    void                    graph_delete(Graph*, Node* node); // remove, deinit, and release memory
    void                    graph_clear(Graph*); // Delete all nodes, wires, edges and reset scope.
    void                    graph_reset(Graph*); // clear + add root node
    bool                    graph_update(Graph*);
    Node*                   graph_root(Graph*);
    const Node*             graph_root(const Graph*);
    Scope*                  graph_root_scope(const Graph*);
    bool                    graph_is_empty(const Graph*);
    void                    graph_connect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags);
    void                    graph_connect_to_variable(Node_Slot* output_slot, Node* variable );
    void                    graph_connect_or_merge(Node_Slot* tail, Node_Slot* head);
    void                    graph_disconnect(Node_Slot* tail, Node_Slot* head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_transfer_children(Scope* /* from */, Scope* /* to */);
    void                    graph_change_node_scope(Node*, Scope* /*desired_scope*/);
    bdc::Array<Scope *>     graph_collect_scopes(const Graph*);
    bdc::Array<Scope *>     graph_collect_root_scopes(const Graph*);
    void                    graph_flag_node_to_delete(Node*, Graph_Flags = Graph_Flag_NONE);
    bool                    graph_contains(const Graph*, Node*);

    // Pointers to/from ID conversion procedures

    Node*                   graph_find_node(Graph*, Node_ID);

    Node*                   graph_get_node(Graph*, Node_ID);
    Node_Slot*              graph_get_slot(Graph*, Node_Slot_ID);
    Scope*                  graph_get_scope(Graph*, Scope_ID);
    
    Node_ID                 graph_get_node_id(Graph*, const Node*);
    Node_Slot_ID            graph_get_slot_id(Graph*, const Node_Slot*);
    Scope_ID                graph_get_scope_id(Graph*, Scope*);

    // ID-based procedures

    Node*                   graph_new_node(Graph*, const Node_Init_Params&);
    void                    graph_delete(Graph*, Node_ID);
    void                    graph_change_node_scope(Graph*, Node_ID, Scope_ID /* new_scope */);
    void                    graph_connect(Graph*, Node_Slot_ID tail, Node_Slot_ID head, Graph_Flags = Graph_Flag_NONE );
    void                    graph_disconnect(Node_Slot_ID tail, Node_Slot_ID head, Graph_Flags = Graph_Flag_NONE );
}