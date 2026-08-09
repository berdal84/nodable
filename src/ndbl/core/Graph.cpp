#include "Graph.h"

#include <algorithm>    // std::find_if
#include <cassert>
#include <cstddef>
#include <imgui/imgui_internal.h>

#include "core/Asserts.h"
#include "core/Node_Slot.h"
#include "language/Nodlang.h"
#include "Node.h"
#include "Scope.h"

using namespace ndbl;
using namespace tools;

Graph::~Graph()
{
    assert(graph_is_empty(this)); // "Did you call graph_deinit() ?\n");
}

void ndbl::graph_init(Graph* graph)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Initializing ...\n");
    ASSERT( graph->nodes.empty() ); // Did you call graph_init multiple times? Did you forgot to call graph_deinit() after each graph_init() ?

    componentbag_init(&graph->component_bag, graph);

    graph_clear(graph);

    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Initialized " TOOLS_OK "\n");
}

void ndbl::graph_deinit(Graph* graph)
{
    graph_clear(graph);

    // Delete each component
    for(auto* component : graph->component_bag)
    {
        component_deinit(component);
        delete component;
    }

    componentbag_deinit(&graph->component_bag);
}

void ndbl::graph_clear(Graph* graph)
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Clearing ...\n");

    // Delete existing nodes
    // (from last to first (which is the root))
    for(auto it = graph->nodes.rbegin(); it != graph->nodes.rend(); ++it)
    {
        Node* node = *it;
        graph_clean_node(node);
        graph->signal_remove_node.emit(node);        
        node_deinit(node);
        delete node;
    }
    graph->nodes.clear();

    // Add a root node
    Node* root_node = new Node();
    node_init_as_root_scope(root_node);
    graph_insert(graph, root_node, nullptr);

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
    std::stack<Node*> node_to_delete; // store pointers to delete all later (avoids to allocate or move data in graph->nodes)
    for(Node* node : graph->nodes)
    {
        if ( node->has_flags(Node_Flag_MUST_BE_DELETED))
        {
            node_to_delete.push(node);
        }
        else if ( node->has_flags(Node_Flag_IS_DIRTY) )
        {
            changed |= node_update(node);
        }
    }

    // Delete flagged nodes
    while( !node_to_delete.empty() )
    {
        changed |= true;
        Node* node = node_to_delete.top();
        graph_clean_node(node);
        node_deinit(node);
        delete node;
        node_to_delete.pop();
    }

    if ( changed )
    {
        graph->signal_change.broadcast();
    }

    return changed;
}

void ndbl::graph_insert(Graph* graph, Node* node, Scope* scope)
{
    // do the inverse of Graph::_erase(Node* node)

    if ( node->scope == nullptr && scope == nullptr && graph->nodes.size() != 0 )
    {
        scope = graph_root_scope(graph);
    }

    if ( scope != nullptr )
    {
        VERIFY( !graph->nodes.empty(), "can't insert a scoped node first, a root node (with a nullptr scope) should be inserted before." );
        VERIFY( node->scope == nullptr, "node must be unscoped, use scope argument instead" );
        VERIFY( scope->node()->graph == graph, "the provided scope belong to another graph" );
        assert(!node->has_flags(Node_Flag_WAS_IN_A_SCOPE_ONCE)); // double-check
        scope_append(scope, node);
    }
    else
    {
        VERIFY( graph->nodes.empty(), "you didn't provided a scope argument, which is only valid for the first insert (root node).");
    }

    node->graph = graph;
	graph->nodes.push_back( node );

    graph->signal_add_node.emit(node);
    graph->signal_change.broadcast();

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- add node %p (name: %s, class: %s)\n", node, node->name.c_str(), node->get_class()->name());
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

Node* ndbl::graph_create_return(Graph* graph, const tools::Type_Descriptor* type_descriptor, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_return(node, type_descriptor);
    graph_insert(graph, node, parent_scope);
    return node;
}

Node* ndbl::graph_create_scope(Graph* graph, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_scope(node);
    graph_insert(graph, node, parent_scope);
    return node;
}

Node* ndbl::graph_create_variable(Graph* graph, const Type_Descriptor *_type, const std::string& _name, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_variable(node, _type, _name.c_str());
    graph_insert(graph, node, parent_scope);
	return node;
}

Node* ndbl::graph_create_function(Graph* graph, const Function_Descriptor& _type, Scope* scope)
{
    Node* node = new Node();
    node_init_as_invokable(node, _type, Node_Type_FUNCTION);
    graph_insert(graph, node, scope);
    return node;
}

Node* ndbl::graph_create_operator(Graph* graph, const Function_Descriptor& _type, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_invokable(node, _type, Node_Type_OPERATOR);
    graph_insert(graph, node, parent_scope);
    return node;
}

void ndbl::graph_find_and_destroy(Graph* graph, Node* node)
{
    if (!node)
        return;

    auto it = std::find(graph->nodes.begin(), graph->nodes.end(), node);
    ASSERT( it != graph->nodes.end() );

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

    graph->nodes.erase(it);
    graph->signal_remove_node.emit(node);
    graph->signal_change.broadcast();

    node_deinit(node);
    delete node;
}

void ndbl::graph_connect_or_merge(Node_Slot* tail, Node_Slot* head )
{
    // Guards
    ASSERT(head->has_flags(Node_Slot::Flag_INPUT ) );
    ASSERT(!head->is_full());
    ASSERT(tail->has_flags(Node_Slot::Flag_OUTPUT ) );
    ASSERT(!tail->is_full());
    VERIFY(head->property, "tail property must be defined" );
    VERIFY(tail->property, "head property must be defined" );
    VERIFY(head->node != tail->node, "Can't connect same primary_child!" );

    // now graph is abstract
//    const type* out_type = __out->property->get_type();
//    const type* in_type  = _in->property->get_type();
//    EXPECT( type::is_implicitly_convertible( out_type, in_type ), "dependency type should be implicitly convertible to dependent type");

    // case 1: merge orphan slot
    if (tail->node == nullptr ) // if dependent is orphan
    {
        property_digest(head->property, tail->property );
        delete head->property;
        // set_dirty(); // no changes on edges/nodes
        return;
    }

    // case 2: merge literals when not connected to a variable
    if (tail->node->type == Node_Type_LITERAL && tail->property->token.word_len() < 16 )
        if (head->node->type != Node_Type_VARIABLE )
        {
            property_digest(head->property, tail->property );
            graph_find_and_destroy(tail->node->graph, tail->node);
            return;
        }

    // Connect (case 4)
    return graph_connect(tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

void ndbl::graph_connect_to_variable(Node_Slot* output_slot, Node* _variable )
{
    // Guards
    ASSERT( output_slot->has_flags(Node_Slot::Flag_OUTPUT) );
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
                    if ( tail->has_flags(Node_Slot::Flag_IS_INTERNAL) )
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
                        if( node_is_conditional(target_scope->node()) )
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
                    std::string buf;
                    get_language()->serialize_default_buffer(buf, token.m_type);
                    token.word_replace( buf.c_str() );
                }
                break;
            }
            default:
                VERIFY(false, "Unexpected _edge.type()");
        }
    }

    tail->node->graph->signal_change.broadcast();
}

Node* ndbl::graph_create_cond_struct(Graph* graph, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_cond_struct(node);
    graph_insert(graph, node, parent_scope);
    return node;
}

Node* ndbl::graph_create_for_loop(Graph* graph, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_for_loop(node);
    graph_insert(graph, node, parent_scope);
    return node;
}

Node* ndbl::graph_create_while_loop(Graph* graph, Scope* parent_scope)
{
    Node* node = new Node();
    node_init_as_while_loop(node);
    graph_insert(graph, node, parent_scope);
    return node;
}

Node* ndbl::graph_create_node(Graph* graph, Scope* scope)
{
    Node* node = new Node();
    
    node_init(node, Node_Type_NULL, "");
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot::Flag_FLOW_IN);

    graph_insert(graph, node, scope);
    
    return node;
}

Node* ndbl::graph_create_literal(Graph* graph, const Type_Descriptor* _type, Scope* scope)
{
    Node* node = new Node();
    node_init_as_literal(node,_type);
    graph_insert(graph, node, scope);
    return node;
}

Node* ndbl::graph_create_node(Graph* graph, Create_Node_Type type, const Function_Descriptor* func_desc, Scope* scope)
{
    switch ( type )
    {
        /*
         * TODO: We could consider narowing the enum to few cases (BLOCK, VARIABLE, LITERAL, OPERATOR, FUNCTION)
         *       and rely more on _signature (ex: a bool variable could be simply "bool" or "bool bool(bool)")
         */
        case Create_Node_Type_BLOCK_CONDITION:  return graph_create_cond_struct(graph, scope);
        case Create_Node_Type_BLOCK_FOR_LOOP:   return graph_create_for_loop(graph, scope);
        case Create_Node_Type_BLOCK_WHILE_LOOP: return graph_create_while_loop(graph, scope);
        case Create_Node_Type_ROOT:             graph_reset(graph);
                                                 return graph_root(graph);

        case Create_Node_Type_VARIABLE_BOOLEAN: return graph_create_variable_decl<bool>(graph, "b", scope);
        case Create_Node_Type_VARIABLE_DOUBLE:  return graph_create_variable_decl<double>(graph, "d", scope);
        case Create_Node_Type_VARIABLE_INTEGER: return graph_create_variable_decl<int>(graph, "i", scope);
        case Create_Node_Type_VARIABLE_STRING:  return graph_create_variable_decl<std::string>(graph, "str", scope);

        case Create_Node_Type_LITERAL_BOOLEAN:  return graph_create_literal<bool>(graph, scope);
        case Create_Node_Type_LITERAL_DOUBLE:   return graph_create_literal<double>(graph, scope);
        case Create_Node_Type_LITERAL_INTEGER:  return graph_create_literal<int>(graph, scope);
        case Create_Node_Type_LITERAL_STRING:   return graph_create_literal<std::string>(graph, scope);

        case Create_Node_Type_RETURN:           return graph_create_return(graph, nullptr, scope);

        case Create_Node_Type_FUNCTION:
        {
            VERIFY(func_desc != nullptr, "_signature is expected when dealing with functions or operators");
            if ( get_language()->is_operator( func_desc ) )
                return graph_create_operator( graph, *func_desc, scope );
            return graph_create_function( graph, *func_desc, scope );
        }
        default:
            TOOLS_UNREACHABLE("Unexpected Create_Node_Type: %i\n", type);
            return nullptr;
    }
}

Node* ndbl::graph_create_variable_ref(Graph* graph, Scope* scope)
{
    Node* node = new Node();
    node_init_as_variable_ref(node);
    graph_insert(graph, node, scope);
    return node;
}

Node* ndbl::graph_create_variable_decl(Graph* graph, const Type_Descriptor* type, const char*  name, Scope* scope)
{
    // Create variable
    Node* var_node = graph_create_variable(graph, type, name, scope);
    var_node->variable_data.set_flags(VariableFlag_DECLARED); // yes, when created from the graph view, variables can be undeclared (== no scope).

    Token token(Token_Type::keyword_operator, " = ");
    token.word_move_begin(1);
    token.word_move_end(-1);
    var_node->variable_data.operator_token = token;

    return var_node;
}

Node* ndbl::graph_create_empty_instruction(Graph* graph, Scope* scope)
{
    Node* node = new Node();
    node_init_as_empty_instruction(node);
    graph_insert(graph, node, scope);
    return node;
}

std::set<Scope *> ndbl::graph_collect_root_scopes(const Graph* graph)
{
    std::set<Scope*> result;
    for (const Node* node : graph->nodes )
        if ( node->internal_scope != nullptr )
            if ( scope_get_depth(node->internal_scope) == 0 )
                result.insert( node->internal_scope );
    return result;
}

std::vector<Scope *> ndbl::graph_collect_scopes(const Graph* graph)
{
    std::vector<Scope *> result;
    for(const Node* node : graph->nodes)
        if ( node->scope )
            result.push_back( node->scope );
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

    node->set_flags(Node_Flag_MUST_BE_DELETED);
}

bool ndbl::graph_contains(const Graph* graph, Node* node)
{
    return std::find( graph->nodes.begin(), graph->nodes.end(), node ) != graph->nodes.end();
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

