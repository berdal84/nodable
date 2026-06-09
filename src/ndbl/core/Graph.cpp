#include "Graph.h"

#include <algorithm>    // std::find_if
#include <imgui/imgui_internal.h>

#include "language/Nodlang.h"
#include "Node.h"
#include "Scope.h"
#include "Node_Slot_Link.h"

using namespace ndbl;
using namespace tools;

Graph::Graph()
: m_components(this)
{
    _init();
}

Graph::~Graph()
{
    _clear();
    m_components.shutdown();
    assert(m_node_registry.empty());
    assert(m_edge_registry.empty());
}

void Graph::_init()
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Initializing ...\n");
    ASSERT( m_node_registry.empty() ); // Root must be first, registry should be empty

    // create and _insert root
    Node* root = new Node();
    node_init_as_root_scope(root);
    this->_insert(root, nullptr);

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- add root node %p (name: %s, class: %s)\n", root, root->name.c_str(), root->get_class()->name());
    ASSERT( root_node() == root );
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Initialized " TOOLS_OK "\n");
}

void Graph::_clear()
{
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Clearing ...\n");

    // delete from last to first (which is the root)
    for(auto it = m_node_registry.rbegin(); it != m_node_registry.rend(); ++it)
    {
        Node* node = *it;
        _clean_node(node);
        _remove(node);        
        node_shutdown(node);
        delete node;
    }

    m_node_registry.clear();

#ifdef NDBL_DEBUG
    if ( !m_edge_registry.empty() )
    {
        TOOLS_LOG(tools::Verbosity_Error, "Graph", "m_edge_registry should be empty.\n" );
        TOOLS_LOG(tools::Verbosity_Message, "Graph", "Dumping %zu edge(s) for debugging purpose ...\n", m_edge_registry.size() );
        for ( auto& edge : m_edge_registry)
        {
            TOOLS_LOG(tools::Verbosity_Message, "Graph", "   %s\n", to_string(edge.second).c_str() );
        }
        m_edge_registry.clear();
    }
#endif

    assert(m_node_registry.empty());
    assert(m_edge_registry.empty());
    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Clear " TOOLS_OK "\n");
}

void Graph::reset()
{
	TOOLS_LOG(tools::Verbosity_Diagnostic,  "Graph", "Resetting ...\n");

    this->_clear();
    this->_init();
    signal_reset.emit();

    TOOLS_LOG(tools::Verbosity_Diagnostic, "Graph", "Reset " TOOLS_OK "\n");
}

bool Graph::update()
{
    bool _changed = false;

    std::stack<Node*> node_to_delete;

    for(Node* node : m_node_registry)
    {
        if ( node->has_flags(Node_Flag_MUST_BE_DELETED))
        {
            node_to_delete.push(node);
        }
        else if ( node->has_flags(Node_Flag_IS_DIRTY) )
        {
            _changed |= node_update(node);
        }
    }

    while( !node_to_delete.empty() )
    {
        _changed |= true;
        Node* node = node_to_delete.top();
        _clean_node(node);
        node_shutdown(node);
        delete node;
        node_to_delete.pop();
    }

    if ( _changed )
    {
        signal_change.broadcast(); // TODO: rather be signal_update, signal_change is already emitted within function calls from this method
    }

    return _changed;
}

void Graph::_insert(Node* node, Scope* scope)
{
    // do the inverse of Graph::_erase(Node* node)

    if ( scope != nullptr )
    {
        VERIFY( !m_node_registry.empty(), "can't insert a scoped node first, a root node (with a nullptr scope) should be inserted before." );
        VERIFY( node->scope == nullptr, "node must be unscoped, use scope argument instead" );
        VERIFY( scope->node()->graph == this, "the provided scope belong to another graph" );
        assert(!node->has_flags(Node_Flag_WAS_IN_A_SCOPE_ONCE)); // double-check
        scope->append(node);
    }
    else
    {
        VERIFY( m_node_registry.empty(), "you didn't provided a scope argument, which is only valid for the first insert (root node).");
    }

    node->graph = this;
	m_node_registry.push_back( node );

    signal_add_node.emit(node);
    signal_change.broadcast();

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- add node %p (name: %s, class: %s)\n", node, node->name.c_str(), node->get_class()->name());
}

void Graph::_remove(Node* node)
{
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- node %p (name: \"%s\"): erasing ...\n", node, node->name.c_str() );
    signal_remove_node.emit(node);
    signal_change.broadcast();
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- node %p (name: \"%s\"): _erased\n", node, node->name.c_str() );
}

void Graph::_clean_node(Node* node)
{
    ASSERT( node );
    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- node %p (name: \"%s\"): pre_erasing ...\n", node, node->name.c_str() );

    // disconnect and erase any link related to this node
    auto concerns_node = [&](const std::pair<Node_Slot_Flags, Node_Slot_Link>& pair )
    {
        const Node_Slot_Link& edge = pair.second;
        return edge.tail->node == node
               || edge.head->node == node;
    };    
    auto it = m_edge_registry.begin();
    while( it != m_edge_registry.end() )
    {
        it = std::find_if(it, m_edge_registry.end(), concerns_node);
        if ( it != m_edge_registry.end() )
        {
            Node_Slot_Link& edge = it->second;
            disconnect(edge);
            it = remove(it); // TODO: reconsider this: is it performant? should we erase all at once? probably...
        }
    }

    // unset scope
    if ( node->scope )
    {
        _reset_scope(node);
    }

    // transfer children to default scope
    if (Scope* _internal_scope = node->internal_scope )
    {
        _transfer_children( _internal_scope, root_scope());
    }

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "-- node %p (name: \"%s\"): pre__erased\n", node, node->name.c_str() );
}

Node* Graph::create_scope(Scope* scope)
{
    Node* node = new Node();
    node_init_as_scope(node);
    _insert(node, scope);
    return node;
}

Node* Graph::create_variable(const Type_Descriptor *_type, const std::string& _name, Scope* scope)
{
    Node* node = new Node();
    node_init_as_variable(node, _type, _name.c_str());
    _insert(node, scope);
	return node;
}

Node* Graph::create_function(const Function_Descriptor& _type, Scope* scope)
{
    Node* node = new Node();
    node_init_as_invokable(node, _type, Node_Type_FUNCTION);
    _insert(node, scope);
    return node;
}

Node* Graph::create_operator(const Function_Descriptor& _type, Scope* scope)
{
    Node* node = new Node();
    node_init_as_invokable(node, _type, Node_Type_OPERATOR);
    _insert(node, scope);
    return node;
}

void Graph::find_and_destroy(Node* node)
{
    if (!node)
        return;

    auto it = std::find(m_node_registry.begin(), m_node_registry.end(), node);
    ASSERT( it != m_node_registry.end() );

    // backup slots
    const Node_Slot* flow_in  = node->flow_in();
    const Node_Slot* flow_out = node->flow_out();
    const bool flow_can_be_maintained = flow_in->adjacent_count() == 1
                                     && flow_out->adjacent_count() == 1;
    Node_Slot* prev_adjacent_slot = flow_in->first_adjacent();
    Node_Slot* next_adjacent_slot = flow_out->first_adjacent();

    _clean_node(node); // flow_in/out will be cleared

    // try to maintain flow
    if ( flow_can_be_maintained )
    {
        connect(prev_adjacent_slot, next_adjacent_slot, Graph_Flag_ALLOW_SIDE_EFFECTS );
    }

    _remove(node);
    m_node_registry.erase(it);
    node_shutdown(node);

    delete node;
}

Node_Slot_Link Graph::connect_or_merge(Node_Slot* tail, Node_Slot* head )
{
    // Guards
    ASSERT(head->has_flags(Node_Slot_Flag_INPUT ) );
    ASSERT(!head->is_full());
    ASSERT(tail->has_flags(Node_Slot_Flag_OUTPUT ) );
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
        head->property->digest(tail->property );
        delete head->property;
        // set_dirty(); // no changes on edges/nodes
        return {};
    }

    // case 2: merge literals when not connected to a variable
    if (tail->node->type == Node_Type_LITERAL && tail->property->token().word_len() < 16 )
        if (head->node->type != Node_Type_VARIABLE )
        {
            head->property->digest(tail->property );
            find_and_destroy(tail->node);
            return {};
        }

    // Connect (case 4)
    return connect(tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

Node_Slot_Link Graph::connect_to_variable(Node_Slot* output_slot, Node* _variable )
{
    // Guards
    ASSERT( output_slot->has_flags(Node_Slot_Flag_OUTPUT) );
    ASSERT( !output_slot->is_full() );
    return connect_or_merge( output_slot, _variable->value_in() );
}

void Graph::connect(const std::set<Node_Slot*>& tails, Node_Slot* head, Graph_Flags _flags)
{
    if ( !tails.empty() )
        for (Node_Slot* _tail : tails )
            connect(_tail, head, Graph_Flag_ALLOW_SIDE_EFFECTS );
}

Node_Slot_Link Graph::connect(Node_Slot* tail, Node_Slot* head, Graph_Flags _flags)
{
    // Create and _insert edge
    auto it = m_edge_registry.emplace(tail->type(), Node_Slot_Link{tail, head});
    Node_Slot_Link& edge = it->second;


    // DirectedEdge is just data, we must add manually cross-references to each end of the edge
    edge.tail->add_adjacent( edge.head );
    edge.head->add_adjacent( edge.tail );

    // Handle side effects
    if (_flags & Graph_Flag_ALLOW_SIDE_EFFECTS )
    {
        switch ( edge.type() )
        {
            case Node_Slot_Flag_TYPE_FLOW:
                _handle_connect_flow_side_effects(edge);  break;
            case Node_Slot_Flag_TYPE_VALUE:
                _handle_connect_value_side_effects(edge); break;
            default:
                ASSERT(false);// This connection type is not yet implemented
        }
    }

    signal_change.broadcast();

    TOOLS_DEBUG_LOG(tools::Verbosity_Diagnostic, "Graph", "New edge added\n");

    return edge;
}

void Graph::_handle_connect_value_side_effects(const Node_Slot_Link& edge )
{
    // ensure the tail node has the right scope
    // must be:
    // - unchanged in case of a node already part of the code flow
    // - or: head node's scope / internal scope if any
    if ( !node_has_flow_adjacent(edge.tail->node) )
    {
        Node*  tail_node    = edge.tail->node;
        Node*  head_node    = edge.head->node;
        Scope* target_scope = head_node->scope;

        if ( head_node->internal_scope != nullptr )
        {
            target_scope = head_node->internal_scope;
        }

        _change_scope(tail_node, target_scope);
    }

    // make sure head property type matches with tail, update head when needed.
    if ( edge.head->node->type != Node_Type_VARIABLE )
    {
        const Node_Property* tail_prop = edge.tail->property;
        Node_Property* head_prop = edge.head->property;
        head_prop->set_type( tail_prop->get_type() );
    }
}

void Graph::_handle_disconnect_value_side_effects(const Node_Slot_Link& edge )
{
    ASSERT( edge.tail->type_and_order() == Node_Slot_Flag_OUTPUT );

    // reset token to a default value to preserve a correct serialization
    if (edge.head->node->type != Node_Type_VARIABLE )
    {
        Token& tok = edge.head->property->token();
        std::string buf;
        get_language()->serialize_default_buffer(buf, tok.m_type);
        tok.word_replace( buf.c_str() );
    }
}

void Graph::_handle_disconnect_flow_side_effects(const Node_Slot_Link& edge )
{
    ASSERT( edge.tail->type_and_order() == Node_Slot_Flag_FLOW_OUT );

    // Ensure disconnected node gets in the right scope
    //
    Scope* target_scope = root_scope();
    switch ( edge.head->adjacent_count())
    {
        case 0:
            break;
        case 1:
        {
            target_scope = edge.head->first_adjacent_node()->scope;
            break;
        }
        default: // 2+
        {
            // Find the lowest common ancestor of adjacent node(s)
            std::set<Scope*> scopes;
            for(Node_Slot* _adjacent_slot : edge.head->adjacent() )
                scopes.insert(_adjacent_slot->node->scope);
            Scope* ancestor = Scope::lowest_common_ancestor(scopes);

            if ( ancestor != nullptr )
            {
                ASSERT( ancestor->parent() != nullptr );
                target_scope = ancestor->parent();
                ASSERT(false); // TODO: here we must create a flow edge from the ancestor's node to edge.head->node
            }
        }
    }
    _change_scope(edge.head->node, target_scope);
}

void Graph::_handle_connect_flow_side_effects(const Node_Slot_Link& edge )
{
    ASSERT( edge.tail->type_and_order() == Node_Slot_Flag_FLOW_OUT );

    Node*  previous_node      = edge.tail->node;
    Node*  next_node          = edge.head->node;
    size_t    flow_in_edge_count = edge.head->adjacent_count();

    if ( flow_in_edge_count == 1)
    {
        if ( edge.tail->has_flags(Node_Slot_Flag_IS_INTERNAL) )
        {
            Scope* target_scope = previous_node->internal_scope;
            if ( node_is_conditional(previous_node) )
            {
                if( next_node->internal_scope == nullptr && !node_is_connected_to_codeflow(next_node) )
                {
                    // insert a scope between target_scope and next_node
                    Node* intermediate_node = create_scope(target_scope);
                    target_scope->reset_head(intermediate_node);
                    target_scope = intermediate_node->internal_scope;
                }
            }
            _change_scope(next_node, target_scope);
            target_scope->reset_head(next_node); // since slot has IS_BRANCH, this node must become the head
        }
        else
        {
            _change_scope(next_node, previous_node->scope);
        }
    }
    else if ( flow_in_edge_count > 1 )
    {
        // gather adjacent scopes
        std::set<Scope*> scopes;
        for(Node_Slot* adjacent : edge.head->adjacent() )
            scopes.insert(adjacent->node->scope );

        if (scopes.size() == 1 )
        {
            _change_scope(next_node, *scopes.begin());
        }
        else
        {
            Scope* target_scope = Scope::lowest_common_ancestor(scopes );
            if( node_is_conditional(target_scope->node()) )
            {
                // We don't want to add a node in a conditional scope, we must pick the parent
                target_scope = target_scope->parent();
            }
            _change_scope(next_node, target_scope);
            // node: no need to branch_scope->reset_head(next_node) here, since when we have 2 flow in or more, we can't be the head
        }
    }
    else
    {
        VERIFY(false, "Unexpected edge count");
    }
}

Edge_Registry::iterator Graph::find(const Node_Slot_Link& edge, Graph_Flags flags)
{
    auto [range_begin, range_end] = m_edge_registry.equal_range( edge.type() & ~Node_Slot_Flag_TYPE_MASK);
    return std::find_if(
            range_begin,
            range_end,
            [&](const auto& _pair) -> bool
            {
                return edge == _pair.second;
            });
}

Edge_Registry::iterator Graph::remove(Edge_Registry::iterator it)
{
    return m_edge_registry.erase(it);
}

void Graph::disconnect(Node_Slot_Link& _edge, Graph_Flags flags)
{
    // disconnect the slots
    _edge.tail->remove_adjacent(_edge.head);
    _edge.head->remove_adjacent(_edge.tail);

    // handle side effects
    if ( flags & Graph_Flag_ALLOW_SIDE_EFFECTS )
    {
        switch ( _edge.type() )
        {
            case Node_Slot_Flag_TYPE_FLOW:
            {
                _handle_disconnect_flow_side_effects(_edge);
                break;
            }
            case Node_Slot_Flag_TYPE_VALUE:
            {
                _handle_disconnect_value_side_effects(_edge);
                break;
            }
            default:
                VERIFY(false, "Unexpected _edge.type()");
        }

        auto it = find(_edge, flags);
        VERIFY(it !=  m_edge_registry.end(), "You're trying to disconnect an edge that is not registered! Did you run this twice?");
        m_edge_registry.erase(it);
    }

    signal_change.broadcast();
}

Node* Graph::create_cond_struct(Scope* scope)
{
    Node* node = new Node();
    node_init_as_cond_struct(node);
    _insert(node, scope);
    return node;
}

Node* Graph::create_for_loop(Scope* scope)
{
    Node* node = new Node();
    node_init_as_for_loop(node);
    _insert(node, scope);
    return node;
}

Node* Graph::create_while_loop(Scope* scope)
{
    Node* node = new Node();
    node_init_as_while_loop(node);
    _insert(node, scope);
    return node;
}

Node* Graph::create_node(Scope* scope)
{
    Node* node = new Node();
    
    node_init(node, Node_Type_NULL, "");
    node_add_slot(node, node->value, Node_Slot_Flag_FLOW_OUT, 1);
    node_add_slot(node, node->value, Node_Slot_Flag_FLOW_IN);

    _insert(node, scope);
    
    return node;
}

Node* Graph::create_literal(const Type_Descriptor* _type, Scope* scope)
{
    Node* node = new Node();
    node_init_as_literal(node,_type);
    _insert(node, scope);
    return node;
}

Node* Graph::create_node(Create_Node_Type_ _type, const Function_Descriptor* _signature, Scope* scope)
{
    switch ( _type )
    {
        /*
         * TODO: We could consider narowing the enum to few cases (BLOCK, VARIABLE, LITERAL, OPERATOR, FUNCTION)
         *       and rely more on _signature (ex: a bool variable could be simply "bool" or "bool bool(bool)")
         */
        case Create_Node_Type__BLOCK_CONDITION:  return create_cond_struct(scope);
        case Create_Node_Type__BLOCK_FOR_LOOP:   return create_for_loop(scope);
        case Create_Node_Type__BLOCK_WHILE_LOOP: return create_while_loop(scope);
        case Create_Node_Type__ROOT:
            reset(); return root_node();

        case Create_Node_Type__VARIABLE_BOOLEAN: return create_variable_decl<bool>("b", scope);
        case Create_Node_Type__VARIABLE_DOUBLE:  return create_variable_decl<double>("d", scope);
        case Create_Node_Type__VARIABLE_INTEGER: return create_variable_decl<int>("i", scope);
        case Create_Node_Type__VARIABLE_STRING:  return create_variable_decl<std::string>("str", scope);

        case Create_Node_Type__LITERAL_BOOLEAN:  return create_literal<bool>(scope);
        case Create_Node_Type__LITERAL_DOUBLE:   return create_literal<double>(scope);
        case Create_Node_Type__LITERAL_INTEGER:  return create_literal<int>(scope);
        case Create_Node_Type__LITERAL_STRING:   return create_literal<std::string>(scope);

        case Create_Node_Type__FUNCTION:
        {
            VERIFY(_signature != nullptr, "_signature is expected when dealing with functions or operators");
            if ( get_language()->is_operator( _signature ) )
                return create_operator( *_signature, scope );
            return create_function( *_signature, scope );
        }
        default:
            VERIFY(false, "Unhandled Create_Node_Type_.");
            return nullptr;
    }
}

Node* Graph::create_variable_ref(Scope* scope)
{
    Node* node = new Node();
    node_init_as_variable_ref(node);
    _insert(node, scope);
    return node;
}

Node* Graph::create_variable_decl(const Type_Descriptor* type, const char*  name, Scope* scope)
{
    // Create variable
    Node* var_node = create_variable(type, name, scope);
    var_node->variable_data().set_flags(VariableFlag_DECLARED); // yes, when created from the graph view, variables can be undeclared (== no scope).

    Token token(Token_Type::keyword_operator, " = ");
    token.word_move_begin(1);
    token.word_move_end(-1);
    var_node->variable_data().operator_token = token;

    return var_node;
}

Node *Graph::create_empty_instruction(Scope* scope)
{
    Node* node = new Node();
    node_init_as_empty_instruction(node);
    _insert(node, scope);
    return node;
}

std::set<Scope *> Graph::root_scopes()
{
    std::set<Scope*> result;
    for ( Node* node : m_node_registry )
        if ( node->internal_scope != nullptr )
            if ( node->internal_scope->depth() == 0 )
                result.insert( node->internal_scope );
    return result;
}

std::vector<Scope *> Graph::scopes()
{
    std::vector<Scope *> result;
    for(Node* node : m_node_registry)
        if ( node->scope )
            result.push_back( node->scope );
    return result;
}

void Graph::flag_node_to_delete(Node *node, Graph_Flags flags)
{
    ASSERT(node->graph == this);

    if ( flags & Graph_Flag_ALLOW_SIDE_EFFECTS )
    {
        // delete inputs when they share the same scope
        for ( auto input : node->inputs() )
            if ( node->scope == input->scope )
                flag_node_to_delete(input, flags);

        // delete children
        if ( Scope* scope = node->internal_scope )
            for ( Node* _child : scope->children() )
                flag_node_to_delete(_child, flags);
    }

    node->set_flags(Node_Flag_MUST_BE_DELETED);
}

Scope* Graph::root_scope() const
{
    return root_node()->internal_scope;
}

bool Graph::contains(Node* node) const
{
    return std::find( m_node_registry.begin(), m_node_registry.end(), node ) != m_node_registry.end();
}


void Graph::_change_scope(Node* node, Scope* desired_scope)
{
    Scope* current_scope = node->scope;

    VERIFY( current_scope != nullptr, "node must be scoped to be changed for another scope");
    VERIFY( desired_scope != nullptr, "a non null desired scope is expected");

    if ( desired_scope == current_scope )
    {
        return;
    }

    current_scope->remove(node);
    desired_scope->append(node);

    signal_change_scope.emit(node, current_scope, desired_scope);
}

void Graph::_transfer_children(Scope* source, Scope* target)
{
    ASSERT(source);
    ASSERT(target);

    std::set<Node*> child_copy{source->children()};
    for(Node* _child : child_copy)
    {
        _change_scope(_child, target);
        ASSERT(_child->scope == target);
    }

    ASSERT(source->empty());
}

void Graph::_reset_scope(Node* node)
{
    ASSERT(node->scope);
    node->scope->remove(node);
}
