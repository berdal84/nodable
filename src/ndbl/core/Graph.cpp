#include "Graph.h"

#include <cassert>
#include <cstddef>
#include <imgui/imgui_internal.h>
#include <vector>

#include "bdc/String_Hash.hpp"
#include "bdc/String.hpp"
#include "core/GUID.h"
#include "core/Asserts.h"
#include "core/Flags.h"
#include "core/Node_Slot.h"
#include "bdc/Types.hpp"
#include "language/Nodlang.h"
#include "Node.h"
#include "Scope.h"
#include "ndbl/gui/Graph_View.h"

// private
namespace ndbl
{
    Node*   _graph_new_node(Graph*);
    void    _graph_add_node(Graph*, Node*, Scope*); // TODO: merge this with add_node_to_index?
    void    _graph_add_node_to_index(Graph*, Node*, size_t position);
    void    _graph_remove_node_from_index(Graph*, Node*);
}

void ndbl::graph_init(Graph* graph)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Initializing ...\n");
    ASSERT( graph->nodes.size == 0 ); // Did you call graph_init multiple times? Did you forgot to call graph_deinit() after each graph_init() ?

    hashmap_init(graph->node_index_by_id);

    graph_clear(graph);

    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Initialized " TOOLS_OK "\n");
}

void ndbl::graph_deinit(Graph* graph)
{
    graph_clear(graph);
    hashmap_release(graph->node_index_by_id);
    
    if( graph->view )
    {
        graphview_deinit( graph->view );
        bdc::memory_free( graph->view );
    }
}

void ndbl::graph_clear(Graph* graph)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Clearing ...\n");

    // Delete existing nodes
    // (from last to first (which is the root))
    for(u32_t i = graph->nodes.size; i > 0; --i)
    {
        Node& node = graph->nodes[i-1];
        graph_clean_node(&node);
        _graph_remove_node_from_index(graph, &node);       
        node_deinit(&node);
    }

    array_resize(graph->nodes, 0);

    // Add a root node
    Node* root_node = _graph_new_node(graph);
    node_init_as_root_scope(root_node);
    _graph_add_node(graph, root_node, nullptr);

    // notify
    graph->signal_change.broadcast();

    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Clear " TOOLS_OK "\n");
}

void ndbl::graph_reset(Graph* graph)
{
	TOOLS_LOG(tools::Verbosity_Diagnostic,  "Graph", "Resetting ...\n");

    graph_clear(graph);    
    graph->signal_reset.emit();

    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Reset " TOOLS_OK "\n");
}

bool ndbl::graph_update(Graph* graph)
{
    bool changed = false;

    // Update nodes
    std::vector<size_t> node_pos_to_delete; // store location to delete all later (avoids to allocate or move data in graph->nodes)
    size_t i = 0;
    for(Node&node : graph->nodes)
    {
        if ( HAS_FLAGS(node.flags, Node_Flag_MUST_BE_DELETED) )
        {
            node_pos_to_delete.push_back(i);
        }
        else if ( HAS_FLAGS(node.flags, Node_Flag_IS_DIRTY) )
        {
            changed |= node_update(&node);
        }
        ++i;
    }

    // Delete flagged nodes
    // Note: it is important to delete from the end to the begin of the list to avoid invalidating the positions
    for( auto it = node_pos_to_delete.rbegin(); it != node_pos_to_delete.rend(); ++it)
    {
        changed |= true;
        Node* node = &graph->nodes[*it];
        graph_clean_node(node);
        _graph_remove_node_from_index(graph, node);
        node_deinit(node);
    }

    if ( changed )
    {
        graph->signal_change.broadcast();
    }

    return changed;
}

void ndbl::_graph_add_node(Graph* graph, Node* node, Scope* scope)
{
    // do the inverse of Graph::_erase(Node* node)

    if ( node->scope == nullptr && scope == nullptr && (&graph->nodes[0] != node) )
    {
        scope = graph_root_scope(graph);
    }

    if( !scope )
    {
        VERIFY( &graph->nodes[0] == node, "you didn't provided a scope argument, which is only valid for the first insert (root node).");
    }
    else
    {
        VERIFY( node->scope == nullptr, "node must be unscoped, use scope argument instead" );
        VERIFY( scope->node->graph == graph, "the provided scope belong to another graph" );
        ASSERT(!HAS_FLAGS(node->flags, Node_Flag_WAS_IN_A_SCOPE_ONCE)); // double-check
        scope_append(scope, node);
    }

    node->graph = graph;
    _graph_add_node_to_index(graph, node, graph->nodes.size -1);

    graph->signal_add_node.emit(node);
    graph->signal_change.broadcast();

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- add node %p (name: %s, class: %s)\n", node, node->name.c_str(), node->get_class()->name.c_str() );
}

void ndbl::_graph_add_node_to_index(Graph* graph, Node* node, size_t position)
{
    node->id = string_hash( get_next_GUID("node") );
    bdc::hashmap_add(graph->node_index_by_id, node->id, position );

    graph->signal_add_node.emit(node); 
}

void ndbl::_graph_remove_node_from_index(Graph* graph, Node* node)
{
    bdc::hashmap_remove(graph->node_index_by_id, node->id );
    
    graph->signal_remove_node.emit(node); 
}

ndbl::Node* ndbl::graph_find_node(Graph* graph, const bdc::String_Hash& id)
{
    auto result = hashmap_find(graph->node_index_by_id, id);
    if ( !result.ok )
    {
        return nullptr;
    }

    return &graph->nodes[*result.value];
}

void ndbl::graph_clean_node(Node* node)
{
    ASSERT( node );
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- node %p (name: \"%s\"): pre_erasing ...\n", node, node->name.c_str() );

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
    if (Scope* _internal_scope = node->internal_scope )
    {
        graph_transfer_children( _internal_scope, graph_root_scope(node->graph));
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- node %p (name: \"%s\"): pre__erased\n", node, node->name.c_str() );
}

ndbl::Node* ndbl::_graph_new_node(Graph* graph)
{
    ASSERT(graph->nodes.size < NODE_MAX_COUNT);

    Node& node = array_append(graph->nodes, {} );
    node.id = string_hash( get_next_GUID("Node") );

    return &node;
}

ndbl::Node* ndbl::graph_create_return(Graph* graph, const tools::Type_Descriptor* type_descriptor, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_return(node, type_descriptor);
    _graph_add_node(graph, node, parent_scope);
    return node;
}

ndbl::Node* ndbl::graph_create_scope(Graph* graph, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_scope(node);
    _graph_add_node(graph, node, parent_scope);
    return node;
}

ndbl::Node* ndbl::graph_create_variable(Graph* graph, const tools::Type_Descriptor *_type, const bdc::String& _name, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_variable(node, _type, _name.c_str());
    _graph_add_node(graph, node, parent_scope);
	return node;
}

ndbl::Node* ndbl::graph_create_function(Graph* graph, const Type_Descriptor* function_type, Scope* scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_invokable(node, function_type, Node_Type_FUNCTION);
    _graph_add_node(graph, node, scope);
    return node;
}

ndbl::Node* ndbl::graph_create_operator(Graph* graph, const Type_Descriptor* function_type, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_invokable(node, function_type, Node_Type_OPERATOR);
    _graph_add_node(graph, node, parent_scope);
    return node;
}

void ndbl::graph_find_and_destroy_node(Graph* graph, Node* node)
{
    if (!node)
        return;

    auto found = graph_find_node(graph, node->id);
    ASSERT( found );

    // backup slots
    const Node_Slot* flow_in  = node->flow_in();
    const Node_Slot* flow_out = node->flow_out();
    const bool flow_can_be_maintained = flow_in->adjacent.size == 1
                                     && flow_out->adjacent.size == 1;
    Node_Slot* prev_adjacent_slot = flow_in->first_adjacent();
    Node_Slot* next_adjacent_slot = flow_out->first_adjacent();

    graph_clean_node(node); // flow_in/out will be cleared

    // try to maintain flow
    if ( flow_can_be_maintained )
    {
        graph_connect(prev_adjacent_slot, next_adjacent_slot, Graph_Flag_ALLOW_SIDE_EFFECTS );
    }

    _graph_remove_node_from_index(graph, node);
    graph->signal_change.broadcast();
    node_deinit(node);
}

void ndbl::graph_connect_or_merge(Node_Slot* tail, Node_Slot* head )
{
    // Guards
    ASSERT(tail != nullptr);
    ASSERT(head != nullptr);
    ASSERT(HAS_FLAGS(head->flags, Node_Slot::Flag_INPUT ) );
    ASSERT(!head->is_full());
    ASSERT(HAS_FLAGS(tail->flags, Node_Slot::Flag_OUTPUT ) );
    ASSERT(!tail->is_full());
    VERIFY(head->property, "tail property must be defined" );
    VERIFY(tail->property, "head property must be defined" );
    VERIFY(head->node != tail->node, "Can't connect same primary_child!" );

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
    if (tail->node->type == Node_Type_LITERAL && tail->property->token.word_size < 16 )
        if (head->node->type != Node_Type_VARIABLE )
        {
            property_digest(head->property, tail->property );
            graph_find_and_destroy_node(tail->node->graph, tail->node);
            return;
        }

    // Connect (case 4)
    return graph_connect(tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::graph_connect_to_variable(Node_Slot* output_slot, Node* _variable )
{
    // Guards
    ASSERT( HAS_FLAGS(output_slot->flags, Node_Slot::Flag_OUTPUT) );
    ASSERT( !output_slot->is_full() );
    return graph_connect_or_merge( output_slot, _variable->value_in() );
}

void ndbl::graph_connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags)
{
    if ( !tails.empty() )
        for (Node_Slot* _tail : tails )
            graph_connect(_tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::graph_connect(Node_Slot* tail, Node_Slot* head, Graph_Flags _flags)
{
    ASSERT(tail != nullptr);
    ASSERT(head != nullptr);

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
                ASSERT( tail->type_and_order() == Node_Slot::Flag_FLOW_OUT );

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
                                Node* intermediate_node = graph_create_scope(tail->node->graph, target_scope);
                                scope_reset_head(target_scope, intermediate_node);
                                target_scope = intermediate_node->internal_scope;
                            }
                        }
                        graph_change_scope(next_node, target_scope);
                        scope_reset_head(target_scope, next_node); // since slot has IS_BRANCH, this node must become the head
                    }
                    else
                    {
                        graph_change_scope(next_node, previous_node->scope);
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
                        graph_change_scope(next_node, *scopes.begin());
                    }
                    else
                    {
                        Scope* target_scope = scope_find_lowest_common_ancestor(scopes);
                        if( node_is_conditional(target_scope->node) )
                        {
                            // We don't want to add a node in a conditional scope, we must pick the parent
                            target_scope = target_scope->parent;
                        }
                        graph_change_scope(next_node, target_scope);
                        // node: no need to branch_scope->reset_head(next_node) here, since when we have 2 flow in or more, we can't be the head
                    }
                }
                else
                {
                    VERIFY(false, "Unexpected edge count");
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

                    graph_change_scope(tail->node, target_scope);
                }

                // make sure head property type matches with tail, update head when needed.
                if ( head->node->type != Node_Type_VARIABLE )
                {
                    property_set_type(head->property, tail->property->type );
                }
                break;
            }
            default:
                TOOLS_UNREACHABLE("This connection type is not yet implemented");
        }
    }

    tail->node->graph->signal_change.broadcast();

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "New edge added\n");
}

void ndbl::graph_disconnect(Node_Slot* tail, Node_Slot* head, Graph_Flags flags)
{
    ASSERT_DEBUG_ONLY(tail->type() == head->type());

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
                ASSERT( tail->type_and_order() == Node_Slot::Flag_FLOW_OUT );
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
                        ASSERT( ancestor->parent != nullptr );
                        target_scope = ancestor->parent;
                        ASSERT(false); // TODO: here we must create a flow edge from the ancestor's node to edge.head->node
                    }
                }
                graph_change_scope(head->node, target_scope);
                break;
            }

            case Node_Slot::Flag_TYPE_VALUE:
            {
                ASSERT(tail->type_and_order() == Node_Slot::Flag_OUTPUT );

                // reset token to a default value to preserve a correct serialization
                if (head->node->type != Node_Type_VARIABLE )
                {
                    Token& token = head->property->token;
                    bdc::String token_type_as_str = lang_serialize_token_type_default(language(), token.type);
                    token.replace_word( token_type_as_str.c_str() );
                }
                break;
            }
            default:
                VERIFY(false, "Unexpected _edge.type()");
        }
    }

    tail->node->graph->signal_change.broadcast();
}

ndbl::Node* ndbl::graph_create_cond_struct(Graph* graph, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_cond_struct(node);
    _graph_add_node(graph, node, parent_scope);
    return node;
}

ndbl::Node* ndbl::graph_create_for_loop(Graph* graph, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_for_loop(node);
    _graph_add_node(graph, node, parent_scope);
    return node;
}

ndbl::Node* ndbl::graph_create_while_loop(Graph* graph, Scope* parent_scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_while_loop(node);
    _graph_add_node(graph, node, parent_scope);
    return node;
}

ndbl::Node* ndbl::graph_create_node(Graph* graph, Scope* scope)
{
    Node* node = _graph_new_node(graph);
    
    node_init(node, Node_Type_NULL, "");
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);

    _graph_add_node(graph, node, scope);
    
    return node;
}

ndbl::Node* ndbl::graph_create_literal(Graph* graph, const tools::Type_Descriptor* _type, Scope* scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_literal(node,_type);
    _graph_add_node(graph, node, scope);
    return node;
}

ndbl::Node* ndbl::graph_create_node(Graph* graph, const Node_State* node_state, Scope* scope)
{
    using namespace tools;

    //
    // TODO: This function must take a unique struct that is able to create any type of node.
    //

    switch ( node_state->type )
    {
        case Node_Type_IF_ELSE:     return graph_create_cond_struct(graph, scope);
        case Node_Type_FOR_LOOP:    return graph_create_for_loop(graph, scope);
        case Node_Type_WHILE_LOOP:  return graph_create_while_loop(graph, scope);

        case Node_Type_ROOT:
        {
            graph_reset(graph);
            return graph_root(graph);
        }

        case Node_Type_VARIABLE:
        {
            if ( node_state->function_type->function.return_type == type_get<bool>() )
                return graph_create_variable_decl<bool>(graph, "b", scope);

            if ( node_state->function_type->function.return_type == type_get<double>()  )
                return graph_create_variable_decl<double>(graph, "d", scope);

            if ( node_state->function_type->function.return_type == type_get<int>()  )
                return graph_create_variable_decl<int>(graph, "i", scope);

            if ( node_state->function_type->function.return_type == type_get<bdc::String>()  )
                return graph_create_variable_decl<bdc::String>(graph, "str", scope);

            TOOLS_UNREACHABLE("Unexpected function_type!");
        }
        
        case Node_Type_LITERAL:
        {
            if ( node_state->function_type->function.return_type == type_get<bool>()  )
                return graph_create_literal<bool>(graph, scope);   

            if ( node_state->function_type->function.return_type == type_get<double>()  )
                return graph_create_literal<double>(graph, scope);

            if ( node_state->function_type->function.return_type == type_get<int>()  )
                return graph_create_literal<int>(graph, scope);

            if ( node_state->function_type->function.return_type == type_get<bdc::String>()  )
                return graph_create_literal<bdc::String>(graph, scope);

            TOOLS_UNREACHABLE("Unexpected function_type!");
        }
        
        case Node_Type_RETURN:
        {
            return graph_create_return(graph, nullptr, scope);
        }

        case Node_Type_FUNCTION:
        {
            VERIFY(node_state->function_type != nullptr, "_signature is expected when dealing with functions or operators");
            if ( lang_is_operator( language(), node_state->function_type ) )
                return graph_create_operator( graph, node_state->function_type, scope );
            return graph_create_function( graph, node_state->function_type, scope );
        }

        default:
            TOOLS_UNREACHABLE("Unexpected Create_Node_Type: %i\n", node_state->type);
            return nullptr;
    }
}

ndbl::Node* ndbl::graph_create_variable_ref(Graph* graph, Scope* scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_variable_ref(node);
    _graph_add_node(graph, node, scope);
    return node;
}

ndbl::Node* ndbl::graph_create_variable_decl(Graph* graph, const tools::Type_Descriptor* type, const bdc::String  name, Scope* scope)
{
    // Create variable
    Node* var_node = graph_create_variable(graph, type, name, scope);
    SET_FLAGS(var_node->component.variable.flags, VariableFlag_DECLARED); // yes, when created from the graph view, variables can be undeclared (== no scope).

    Token token{
        Token_Type_keyword_operator,
        " = "
    };
    token.word_move_begin(1);
    token.word_move_end(-1);
    var_node->component.variable.operator_token = token;

    return var_node;
}

ndbl::Node* ndbl::graph_create_empty_instruction(Graph* graph, Scope* scope)
{
    Node* node = _graph_new_node(graph);
    node_init_as_empty_instruction(node);
    _graph_add_node(graph, node, scope);
    return node;
}

std::set<ndbl::Scope*> ndbl::graph_collect_root_scopes(const Graph* graph)
{
    std::set<Scope*> result;
    for (const Node& node : graph->nodes )
        if ( node.internal_scope != nullptr )
            if ( scope_get_depth(node.internal_scope) == 0 )
                result.insert( node.internal_scope );
    return result;
}

std::vector<ndbl::Scope*> ndbl::graph_collect_scopes(const Graph* graph)
{
    std::vector<Scope *> result;
    for(const Node& node : graph->nodes)
        if ( node.scope )
            result.push_back( node.scope );
    return result;
}

void ndbl::graph_flag_node_to_delete(Node *node, Graph_Flags flags)
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

    SET_FLAGS(node->flags, Node_Flag_MUST_BE_DELETED);
}

bool ndbl::graph_contains(const Graph* graph, Node* node)
{
    return hashmap_find(graph->node_index_by_id, node->id).ok;
}

ndbl::Node* ndbl::graph_get_latest_created_node(Graph* graph)
{
    return &array_back(graph->nodes);
}

const ndbl::Node* ndbl::graph_get_latest_created_node(const Graph* graph)
{
    return &array_back(graph->nodes);
}

void ndbl::graph_change_scope(Node* node, Scope* desired_scope)
{
    Scope* current_scope = node->scope;

    VERIFY( current_scope != nullptr, "node must be scoped to be changed for another scope");
    VERIFY( desired_scope != nullptr, "a non null desired scope is expected");

    if ( desired_scope == current_scope )
    {
        return;
    }

    scope_remove(current_scope, node);
    scope_append(desired_scope, node);

    ASSERT_DEBUG_ONLY(node->graph != nullptr);
    node->graph->signal_change_scope.emit({
        node, 
        current_scope,
        desired_scope
    });
}

void ndbl::graph_transfer_children(Scope* source, Scope* target)
{
    ASSERT(source);
    ASSERT(target);

    std::set<Node*> child_copy{source->children};
    for(Node* _child : child_copy)
    {
        graph_change_scope(_child, target);
        ASSERT(_child->scope == target);
    }

    ASSERT( scope_is_empty(source) );
}

