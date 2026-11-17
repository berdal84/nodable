#include "Graph.h"

#include <cassert>
#include <cstddef>
#include <imgui/imgui_internal.h>
#include <vector>

#include "bdc/String_Hash.hpp"
#include "bdc/String.hpp"
#include "bdc/Types.hpp"
#include "Asserts.h"
#include "Flags.h"
#include "Graph_View.h"
#include "GUID.h"
#include "Node_Slot.h"
#include "Node.h"
#include "Parser.h"

namespace ndbl
{

using namespace bdc;

void graph_init(Graph* graph)
{
    NDBL_LOG(Verbosity_Diagnostic, "Graph", "Initializing ...\n");
    NDBL_ASSERT( graph->node_by_id.size == 0 ); // Did you call graph_init multiple times? Did you forgot to call graph_deinit() after each graph_init() ?

    graph_reset(graph);

    NDBL_LOG(Verbosity_Diagnostic, "Graph", "Initialized " NDBL_OK "\n");
}

void graph_deinit(Graph* graph)
{
    graph_clear(graph);
    hashmap_release(graph->node_by_id);
}

void graph_clear(Graph* graph)
{
    if( graph->node_by_id.size == 0 )
    {
        return;
    }

    NDBL_LOG(Verbosity_Diagnostic, "Graph", "Resetting ...\n");

    Node* root = graph_root(graph);

    // first, remove all except root
    FOR_EACH_HASHMAP_VALUE(node, graph->node_by_id)
    {
        if( node != root)
        {
            graph_delete(graph, node);
        }
        
    }
    hashmap_clear(graph->node_by_id);

    // delete root last
    graph_delete(graph, root);    
}

void graph_reset(Graph* graph)
{
    NDBL_LOG(Verbosity_Diagnostic, "Graph", "Clearing ...\n");

    graph_clear(graph);

    // Add a root node
    Node* root_node = memory_new<Node>();
    node_init_as_root_scope(root_node);
    graph->root_node_id = root_node->id;
    graph_add(graph, root_node, nullptr);

    // notify
    graph->signal_change.broadcast();
    graph->signal_reset.emit();

    NDBL_LOG(Verbosity_Diagnostic, "Graph", "Clear " NDBL_OK "\n");
}

bool graph_update(Graph* graph)
{
    bool changed = false;

    // Update and delete nodes as necessary
    // Iterate from the end to the beginning to safely update and delete nodes
    FOR_EACH_HASHMAP_VALUE(node, graph->node_by_id)
    {
        if ( HAS_FLAGS(node->flags, Node_Flag_MARKED_FOR_DELETION) )
        {
            changed |= true;
            graph_remove(graph, node);
            node_deinit(node);
            memory_delete(node);
            continue;
        }
    }

    if ( changed )
    {
        graph->signal_change.broadcast();
    }

    return changed;
}

Node* graph_root(Graph* graph)
{
    auto found = hashmap_find(graph->node_by_id, graph->root_node_id);
    NDBL_ASSERT(found.ok, "Unable to find root node!");
    return found.data;
}

const Node* graph_root(const Graph* graph)
{
    auto found = hashmap_find(graph->node_by_id, graph->root_node_id);
    NDBL_ASSERT(found.ok, "Unable to find root node!");
    return found.data;
}

Scope* graph_root_scope(const Graph* graph)
{
    const Node* root_node = graph_root(graph);
    return root_node->internal_scope;
};

bool graph_is_empty(const Graph* graph)
{
    Scope* root_scope = graph_root_scope(graph);
    return scope_is_empty( root_scope );
};

void graph_add(Graph* graph, Node* node, Scope* scope)
{
    // do the inverse of Graph::_erase(Node* node)

    if (
        node->scope == nullptr &&
        scope == nullptr &&
        node->id != graph->root_node_id
    )
    {
        scope = graph_root_scope(graph);
    }

    if( scope == nullptr)
    {
        NDBL_ASSERT(node->id == graph->root_node_id, "you didn't provided a scope argument, which is only valid for the first insert (root node).");
    }
    else
    {
        NDBL_ASSERT( node->scope == nullptr, "node must be unscoped, use scope argument instead" );
        NDBL_ASSERT( scope->node->graph == graph, "the provided scope belong to another graph" );
        NDBL_ASSERT(!HAS_FLAGS(node->flags, Node_Flag_WAS_IN_A_SCOPE_ONCE)); // double-check
        scope_append(scope, node);
    }

    node->graph = graph;
    bdc::hashmap_add(graph->node_by_id, node->id, node);

    graph->signal_add_node.emit(node);
    graph->signal_change.broadcast();

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Graph", "-- add node %p (name: %s, class: %s)\n", node, node->name.c_str(), node->get_class()->name.c_str() );
}

void graph_remove(Graph* graph, Node* node)
{
    assert( node );
    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Graph", "_graph_remove_node(...) - addr: %p, name: \"%s\" ..\n", node, node->name.c_str() );

    graph->signal_remove_node.emit(node);

    // disconnect and erase any link related to this node
    for(Node_Slot* each_slot : node->slots)
        while(each_slot->adjacent.size > 0)
            graph_disconnect(each_slot, each_slot->adjacent[0] );

    // unset scope
    if ( node->scope )
    {
        scope_remove(node->scope, node);
    }

    // transfer children to default scope
    if ( node->id != graph->root_node_id && node->internal_scope != nullptr)
    {
        graph_transfer_children( node->internal_scope, graph_root_scope(node->graph));
    }

    hashmap_remove(graph->node_by_id, node->id);

    if( node->id == graph->root_node_id)
    {
        graph->root_node_id = 0;
        NDBL_ASSERT(graph->node_by_id.size == 0, "Root must be removed last");
        NDBL_ASSERT(node->internal_scope->children.size() == 0, "You should empty children prior to remove");
    }
}

void graph_delete(Graph* graph, Node* node)
{
    if(node == nullptr)
    {
        return;
    }
    
    NDBL_ASSERT(node->graph == graph, "node should belong to this graph");

    // backup slots
    const Node_Slot* flow_in  = node->flow_in();
    const Node_Slot* flow_out = node->flow_out();
    const bool flow_can_be_maintained = flow_in
                                     && flow_in->adjacent.size == 1
                                     && flow_out->adjacent.size == 1;
    Node_Slot* prev_adjacent_slot = flow_in ? flow_in->first_adjacent() : nullptr;
    Node_Slot* next_adjacent_slot = flow_out ? flow_out->first_adjacent() : nullptr;

    // erase
    graph_remove(graph, node);
    node_deinit(node);
    memory_delete(node);

    // try to maintain flow
    if ( flow_can_be_maintained )
    {
        graph_connect(prev_adjacent_slot, next_adjacent_slot, Graph_Flag_ALLOW_SIDE_EFFECTS );
    }

    graph->signal_change.broadcast();
}

void graph_connect_or_merge(Node_Slot* tail, Node_Slot* head )
{
    // Guards
    NDBL_ASSERT(tail != nullptr);
    NDBL_ASSERT(head != nullptr);
    NDBL_ASSERT(HAS_FLAGS(head->flags, Node_Slot::Flag_INPUT ) );
    NDBL_ASSERT(!head->is_full());
    NDBL_ASSERT(HAS_FLAGS(tail->flags, Node_Slot::Flag_OUTPUT ) );
    NDBL_ASSERT(!tail->is_full());
    NDBL_ASSERT(head->property, "tail property must be defined" );
    NDBL_ASSERT(tail->property, "head property must be defined" );
    NDBL_ASSERT(head->node != tail->node, "Can't connect same primary_child!" );

    // now graph is abstract
//    const type* out_type = __out->property->get_type();
//    const type* in_type  = _in->property->get_type();
//    EXPECT( type_is_implicitly_convertible( out_type, in_type ), "dependency type should be implicitly convertible to dependent type");

    // case 1: merge orphan slot
    if (tail->node == nullptr ) // if dependent is orphan
    {
        property_digest(head->property, tail->property );
        bdc::memory_delete(head->property);
        // set_dirty(); // no changes on edges/nodes
        return;
    }

    // case 2: merge literals when not connected to a variable
    if (tail->node->kind == Node_Kind_LITERAL && tail->property->token.word_size < 16 )
        if (head->node->kind != Node_Kind_VARIABLE )
        {
            property_digest(head->property, tail->property );
            graph_delete(tail->node->graph, tail->node);
            return;
        }

    // Connect (case 4)
    return graph_connect(tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void graph_connect_to_variable(Node_Slot* output_slot, Node* _variable )
{
    // Guards
    NDBL_ASSERT( HAS_FLAGS(output_slot->flags, Node_Slot::Flag_OUTPUT) );
    NDBL_ASSERT( !output_slot->is_full() );
    return graph_connect_or_merge( output_slot, _variable->value_in() );
}

void graph_connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags)
{
    if ( !tails.empty() )
        for (Node_Slot* _tail : tails )
            graph_connect(_tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void graph_connect(Node_Slot* tail, Node_Slot* head, Graph_Flags _flags)
{
    NDBL_ASSERT(tail != nullptr);
    NDBL_ASSERT(head != nullptr);

    // DirectedEdge is just data, we must add manually cross-references to each end of the edge
    node_slot_add_adjacent( tail, head );
    node_slot_add_adjacent( head, tail );

    // Handle side effects
    if (_flags & Graph_Flag_ALLOW_SIDE_EFFECTS )
    {
        switch ( tail->type() )
        {
            case Node_Slot::Flag_TYPE_FLOW:
            {
                NDBL_ASSERT( tail->type_and_order() == Node_Slot::Flag_FLOW_OUT );

                Node*  previous_node      = tail->node;
                Node*  next_node          = head->node;
                size_t flow_in_edge_count = head->adjacent.size;

                if ( flow_in_edge_count == 1)
                {
                    if ( HAS_FLAGS(tail->flags, Node_Slot::Flag_IS_INTERNAL) )
                    {
                        Scope* target_scope = previous_node->internal_scope;
                        if ( node_is_conditional(previous_node) )
                        {
                            if( next_node->internal_scope == nullptr && !node_is_connected_to_codeflow(next_node) )
                            {
                                // insert a scope between target_scope and next_node
                                Node* intermediate_node = memory_new<Node>();
                                node_init_as_scope(intermediate_node);
                                graph_add(tail->node->graph, intermediate_node, target_scope);     
                                scope_reset_head(target_scope, intermediate_node);
                                target_scope = intermediate_node->internal_scope;
                            }
                        }
                        graph_change_node_scope(next_node, target_scope);
                        scope_reset_head(target_scope, next_node); // since slot has IS_BRANCH, this node must become the head
                    }
                    else
                    {
                        graph_change_node_scope(next_node, previous_node->scope);
                    }
                }
                else if ( flow_in_edge_count > 1 )
                {
                    // gather adjacent scopes
                    std::set<Scope*> scopes;
                    for(Node_Slot* adjacent : head->adjacent )
                        scopes.insert(adjacent->node->scope );

                    if (scopes.size() == 1 )
                    {
                        graph_change_node_scope(next_node, *scopes.begin());
                    }
                    else
                    {
                        Scope* target_scope = scope_find_lowest_common_ancestor(scopes);
                        if( node_is_conditional(target_scope->node) )
                        {
                            // We don't want to add a node in a conditional scope, we must pick the parent
                            target_scope = target_scope->parent;
                        }
                        graph_change_node_scope(next_node, target_scope);
                        // node: no need to branch_scope->reset_head(next_node) here, since when we have 2 flow in or more, we can't be the head
                    }
                }
                else
                {
                    NDBL_ASSERT(false, "Unexpected edge count");
                }
                break;
            }

            case Node_Slot::Flag_TYPE_VALUE:
            {
                // ensure the tail node has the right scope
                // must be:
                // - unchanged in case of a node already part of the code flow
                // - or: head node's scope / internal scope if any
                if ( !node_has_flow_adjacent(tail->node) )
                {
                    Scope* target_scope = head->node->scope;

                    if ( head->node->internal_scope != nullptr )
                    {
                        target_scope = head->node->internal_scope;
                    }

                    graph_change_node_scope(tail->node, target_scope);
                }

                // make sure head property type matches with tail, update head when needed.
                if ( head->node->kind != Node_Kind_VARIABLE )
                {
                    property_set_type(head->property, tail->property->type );
                }
                break;
            }
            default:
                NDBL_UNREACHABLE("This connection type is not yet implemented");
        }
    }

    tail->node->graph->signal_change.broadcast();

    NDBL_DEBUG_LOG(Verbosity_Diagnostic, "Graph", "New edge added\n");
}

void graph_disconnect(Node_Slot* tail, Node_Slot* head, Graph_Flags flags)
{
    // disconnect the slots
    node_slot_remove_adjacent(tail, head);
    node_slot_remove_adjacent(head, tail);

    // handle side effects
    if ( flags & Graph_Flag_ALLOW_SIDE_EFFECTS )
    {
        switch ( tail->type() )
        {
            case Node_Slot::Flag_TYPE_FLOW:
            {
                NDBL_ASSERT( tail->type_and_order() == Node_Slot::Flag_FLOW_OUT );
                // Ensure disconnected node gets in the right scope
                //
                Scope* target_scope = graph_root_scope(tail->node->graph);
                if( head->adjacent.size == 1)
                {
                    target_scope = head->first_adjacent_node()->scope;
                }
                else if (head->adjacent.size >= 2)
                {
                    // Find the lowest common ancestor of adjacent node(s)
                    std::set<Scope*> scopes;
                    for(Node_Slot* _adjacent_slot : head->adjacent )
                        scopes.insert(_adjacent_slot->node->scope);
                    Scope* ancestor = scope_find_lowest_common_ancestor(scopes);

                    if ( ancestor != nullptr )
                    {
                        NDBL_ASSERT( ancestor->parent != nullptr );
                        target_scope = ancestor->parent;
                        NDBL_ASSERT(false); // TODO: here we must create a flow edge from the ancestor's node to edge.head->node
                    }
                }
                graph_change_node_scope(head->node, target_scope);
                break;
            }

            case Node_Slot::Flag_TYPE_VALUE:
            {
                NDBL_ASSERT(tail->type_and_order() == Node_Slot::Flag_OUTPUT );

                // reset token to a default value to preserve a correct serialization
                if (head->node->kind != Node_Kind_VARIABLE )
                {
                    Serializer_Context ctx;
                    serializer_init(ctx);
                    Token& token = head->property->token;
                    bdc::String token_type_as_str = serialize_token_type(ctx, token.type); // TODO: simply handle literal token_type to string
                    token_replace_word( token, token_type_as_str );
                }
                break;
            }
            default:
                NDBL_ASSERT(false, "Unexpected _edge.type()");
        }
    }

    tail->node->graph->signal_change.broadcast();
}

Array<Scope*> graph_collect_root_scopes(const Graph* graph)
{
    Resizable_Array<Scope *> result;
    result.allocator = &temp_allocator;

    FOR_EACH_HASHMAP_VALUE(node, graph->node_by_id)
        if ( node->internal_scope != nullptr )
            if ( scope_get_depth(node->internal_scope) == 0 )
                array_append( result, node->internal_scope );
    return result;
}

Array<Scope*> graph_collect_scopes(const Graph* graph)
{
    Resizable_Array<Scope *> result;
    result.allocator = &temp_allocator;
    
    FOR_EACH_HASHMAP_VALUE(node, graph->node_by_id)
        if ( node->scope )
            array_append( result, node->scope );
    return result;
}

void graph_flag_node_to_delete(Node *node, Graph_Flags flags)
{
    if ( flags & Graph_Flag_ALLOW_SIDE_EFFECTS )
    {
        // delete inputs when they share the same scope
        for ( auto input : node->inputs() )
            if ( node->scope == input->scope )
                graph_flag_node_to_delete(input, flags);

        // delete children
        if ( Scope* scope = node->internal_scope )
            for ( Node* _child : scope->children )
                graph_flag_node_to_delete(_child, flags);
    }

    SET_FLAGS(node->flags, Node_Flag_MARKED_FOR_DELETION);
}

bool graph_contains(const Graph* graph, Node* node)
{
    return hashmap_find(graph->node_by_id, node->id).ok;
}

void graph_change_node_scope(Node* node, Scope* desired_scope)
{
    Scope* current_scope = node->scope;

    NDBL_ASSERT( current_scope != nullptr, "node must be scoped to be changed for another scope");
    NDBL_ASSERT( desired_scope != nullptr, "a non null desired scope is expected");

    if ( desired_scope == current_scope )
    {
        return;
    }

    scope_remove(current_scope, node);
    scope_append(desired_scope, node);

    NDBL_ASSERT_DEBUG_ONLY(node->graph != nullptr);
    node->graph->signal_change_scope.emit({
        node, 
        current_scope,
        desired_scope
    });
}

void graph_transfer_children(Scope* source, Scope* target)
{
    NDBL_ASSERT(source);
    NDBL_ASSERT(target);

    std::set<Node*> child_copy{source->children};
    for(Node* _child : child_copy)
    {
        graph_change_node_scope(_child, target);
        NDBL_ASSERT(_child->scope == target);
    }

    NDBL_ASSERT( scope_is_empty(source) );
}

Node* graph_new_node(Graph* graph, const Node_Init_Params& params)
{
    Node*  node         = new_node(params);
    Scope* parent_scope = graph_get_scope(graph, params.parent_scope_id);
    graph_add(graph, node, parent_scope);

    return node;    
}

Node* graph_get_node(Graph* graph, Node_ID id)
{
    bdc::Result<Node*> result = hashmap_find(graph->node_by_id, id);

    if ( result.ok )
    {
        return result.data;
    }

    return nullptr;    
}

Scope* graph_get_scope(Graph* graph, Scope_ID id)
{
    Node* node = graph_get_node(graph, id);
    return node ? node->internal_scope : nullptr;
}

void graph_delete(Graph* graph, Node_ID id)
{
    Node* node = graph_get_node(graph, id);
    NDBL_ASSERT(node, "Unable to find node in graph");
    graph_delete(graph, node);
}

} // namespace ndbl