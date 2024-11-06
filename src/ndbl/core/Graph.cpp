#include "Graph.h"

#include <algorithm>    // std::find_if

#include "DirectedEdge.h"
#include "IfNode.h"
#include "LiteralNode.h"
#include "Node.h"
#include "NodeFactory.h"
#include "Scope.h"
#include "VariableNode.h"
#include "language/Nodlang.h"
#include "VariableRefNode.h"

using namespace ndbl;
using namespace tools;

Graph::Graph(NodeFactory* factory)
: m_factory(factory)
{
}

Graph::~Graph()
{
	clear();
}

void Graph::clear()
{
    if ( m_root.empty() && m_node_registry.empty() && m_edge_registry.empty() )
    {
        return;
    }

	LOG_VERBOSE( "Graph", "Clearing graph ...\n");
    while ( !m_node_registry.empty() )
    {
        Node* node = m_node_registry[0];
        LOG_VERBOSE("Graph", "destroying node \"%s\" (id: %zu)\n", node->name().c_str(), (u64_t)node );
        destroy(node);
    }

#ifdef NDBL_DEBUG
    if ( !m_edge_registry.empty() )
    {
        LOG_ERROR("Graph", "m_edge_registry should be empty.\n" );
        LOG_MESSAGE("Graph", "Dumping %zu edge(s) for debugging purpose ...\n", m_edge_registry.size() );
        for ( auto& edge : m_edge_registry)
        {
            LOG_MESSAGE("Graph", "   %s\n", to_string(edge.second).c_str() );
        }
        m_edge_registry.clear();
    }
#endif

    LOG_VERBOSE("Graph", "Graph cleared.\n");
}

bool Graph::update()
{
    bool changed = false;

    // Delete (flagged Nodes) / Check if dirty
    auto nodeIndex = m_node_registry.size();

    while (nodeIndex > 0)
    {
        nodeIndex--;
        Node* node = m_node_registry.at(nodeIndex);

        if (node->has_flags(NodeFlag_TO_DELETE))
        {
            destroy(node);
            changed = true;
        }
        else if ( node->has_flags(NodeFlag_IS_DIRTY) )
        {
            changed = node->update();
        }
    }

    if ( changed )
        on_change.emit();

    return changed;
}

void Graph::add(Node* _node)
{
    ASSERT(std::find(m_node_registry.begin(), m_node_registry.end(), _node) == m_node_registry.end());

	m_node_registry.push_back(_node);
    _node->m_graph = this;

    on_add.emit(_node);
    on_change.emit();

    LOG_VERBOSE("Graph", "add node %s (%s)\n", _node->name().c_str(), _node->get_class()->get_name());
}

void Graph::remove(Node* _node)
{
    auto it = std::find(m_node_registry.begin(), m_node_registry.end(), _node);
    m_node_registry.erase(it);
    on_remove.emit(_node);
    on_change.emit();
}

VariableNode* Graph::create_variable(const TypeDescriptor *_type, const std::string& _name)
{
    VariableNode* node = m_factory->create_variable(_type, _name);
    add(node);
	return node;
}

FunctionNode* Graph::create_function(const FunctionDescriptor* _type)
{
    FunctionNode* node = m_factory->create_function(_type, NodeType_FUNCTION);
    add(node);
    return node;
}

FunctionNode* Graph::create_operator(const FunctionDescriptor* _type)
{
    FunctionNode* node = m_factory->create_function(_type, NodeType_OPERATOR);
    add(node);
    return node;
}

void Graph::destroy(Node* node)
{
    if( node == nullptr )
    {
        return;
    }

    // backup prev/next adjacent slots
    std::vector<Slot*> prev_adjacent_slot;
    if( Slot* slot = node->find_slot(SlotFlag_FLOW_IN) )
        prev_adjacent_slot = slot->adjacent();
    std::vector<Slot*> next_adjacent_slot;
    if ( Slot* slot = node->find_slot(SlotFlag_FLOW_OUT) )
        next_adjacent_slot = slot->adjacent();

    // Identify each edge connected to this node
    std::vector<DirectedEdge> related_edges;
    for(auto& [_type, each_edge]: m_edge_registry)
    {
        if(each_edge.tail->node == node || each_edge.head->node == node )
        {
            related_edges.emplace_back(each_edge);
        }
    }

    // Disconnect all of them
    for(const DirectedEdge& each_edge : related_edges )
    {
        disconnect(each_edge);
    };

    // try to reconnect the successors with the predecessor
    if ( prev_adjacent_slot.size() == 1 && next_adjacent_slot.size() == 1 )
    {
        connect( prev_adjacent_slot[0], next_adjacent_slot[0] );
        // TODO: we must be able to pin the view from here
    }

    // Remove from scope
    if ( Scope* scope = node->parent() )
        scope->remove( node );

    // Remove inner_scope children
    if ( node->is_a_scope() )
        node->internal_scope()->clear();

    // unregister and delete
    remove(node);
    if ( m_root == node )
        m_root.reset();
    m_factory->destroy_node(node);
}

DirectedEdge Graph::connect_or_merge(Slot* tail, Slot* head )
{
    // Guards
    ASSERT(head->has_flags(SlotFlag_INPUT ) );
    ASSERT(head->has_flags(SlotFlag_NOT_FULL ) );
    ASSERT(tail->has_flags(SlotFlag_OUTPUT ) );
    ASSERT(tail->has_flags(SlotFlag_NOT_FULL ) );
    VERIFY(head->property, "tail property must be defined" );
    VERIFY(tail->property, "head property must be defined" );
    VERIFY(head->node != tail->node, "Can't connect same child_node!" );

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
    if (tail->node->type() == NodeType_LITERAL && tail->property->token().word_len() < 16 )
        if (head->node->type() != NodeType_VARIABLE )
        {
            head->property->digest(tail->property );
            destroy(tail->node);
            return {};
        }

    // Connect (case 4)
    return connect(tail, head, ConnectFlag_ALLOW_SIDE_EFFECTS );
}

void Graph::remove(const DirectedEdge& edge)
{
    auto found = std::find_if( m_edge_registry.begin()
                             , m_edge_registry.end()
                             , [edge](auto& each){ return edge == each.second;});

    if (found != m_edge_registry.end() )
    {
        m_edge_registry.erase(found);
    }
    else
    {
        LOG_WARNING("Graph", "Unable to unregister edge\n");
    }
    on_change.emit();
}

DirectedEdge Graph::connect_to_variable(Slot* output_slot, VariableNode* _variable )
{
    // Guards
    ASSERT( output_slot->has_flags(SlotFlag_OUTPUT | SlotFlag_NOT_FULL ) );
    return connect_or_merge( output_slot, _variable->value_in() );
}

void Graph::connect(const std::set<Slot*>& tails, Slot* head, ConnectFlags _flags)
{
    if ( !tails.empty() )
        for (Slot* _tail : tails )
            connect(_tail, head, ConnectFlag_ALLOW_SIDE_EFFECTS );
}

DirectedEdge Graph::connect(Slot* tail, Slot* head, ConnectFlags _flags)
{
    // Create and insert edge
    DirectedEdge edge = add({tail, head});

    // DirectedEdge is just data, we must add manually cross-references to each end of the edge
    edge.tail->add_adjacent( edge.head );
    edge.head->add_adjacent( edge.tail );

    // Handle side effects
    if (_flags & ConnectFlag_ALLOW_SIDE_EFFECTS )
    {
        switch ( edge.type() )
        {
            case SlotFlag_TYPE_FLOW:  on_connect_flow_side_effects(edge);  break;
            case SlotFlag_TYPE_VALUE: on_connect_value_side_effects(edge); break;
            default:
                ASSERT(false);// This connection type is not yet implemented
        }
    }

    on_change.emit();

    LOG_VERBOSE("Graph", "New edge added\n");

    return edge;
}

DirectedEdge Graph::add(const DirectedEdge& _edge)
{
    m_edge_registry.emplace(_edge.type(), _edge);
    on_change.emit();
    return _edge; // copy is OK
}

//void Graph::on_connect_hierarchical_side_effects(Slot* parent_slot, Slot* child_slot)
//{
//    //
//    // This function handle side effects after a new hierarchical (PARENT/CHILD) connection has been made.
//    // It will create one of more codeflow (PREV/NEXT) connection(s) automatically.
//    //
//    Node* parent           = parent_slot->node;
//    Node* new_child        = child_slot->node;
//    Slot* parent_next_slot = parent->find_slot_at(SlotFlag_FLOW_OUT, parent_slot->position );
//
//    ASSERT(parent->has_component<Scope>());
//    ASSERT(parent_next_slot);
//
//    Slot* new_child_prev_slot = new_child->find_slot(SlotFlag_FLOW_IN );
//
//    // Case 1: Parent has only 1 child (the newly connected), we connect it as "next".
//    if ( !parent_next_slot->is_full() )
//    {
//        connect( parent_next_slot, new_child_prev_slot );
//        return;
//    }
//
//    // Case 2: Connects to the last child's "next" slot.
//    //         parent
//    //           - ...
//    //           - last child ->->->
//    //           - new child <-<-<-
//    Node* previous_child = *(parent->children().rbegin() + 1);
//    ASSERT( previous_child );
//
//    // Case 2.a: Connects to all last instructions' "next" slot (in last child's previous_child_scope).
//    //           parent
//    //             - ...
//    //             - previous_child
//    //                  - child 0
//    //                     - ...
//    //                     - instr n >->->->->
//    //                  - child 1
//    //                     - instr 0 >->->->->
//    //                  - ...
//    //                  - instr n ->->->->->->
//    //             - new child <-<-<-<-<-<-<-<
//    //
//    if (Scope* previous_child_scope = previous_child->inner_scope() )
//    {
//        for (Node* each_instr : previous_child_scope->last_instr() )
//        {
//            Slot* each_instr_next_slot = each_instr->find_slot(SlotFlag_FREE_FLOW_OUT );
//            ASSERT(each_instr_next_slot);
//            connect( each_instr_next_slot, new_child_prev_slot );
//        }
//        return;
//    }
//
//    // Case 2.b: Connects to last child's "next" slot.
//    //           parent
//    //             - ...
//    //             - previous_child ->->->
//    //             - new child <-<-<-<-<-<
//    //
//    Slot* last_sibling_next_slot = previous_child->find_slot(SlotFlag_FREE_FLOW_OUT );
//    connect( last_sibling_next_slot, new_child_prev_slot );
//}

void Graph::on_connect_value_side_effects( DirectedEdge edge )
{
    // 1) Update Scope
    //
    Scope* target_scope = edge.head->node->parent();

    if ( edge.head->node->is_a_scope() )
        target_scope = edge.head->node->internal_scope();

    if ( target_scope )
        target_scope->push_back(edge.tail->node ); // recursively


    // 2) Update input's property type
    //
    if ( edge.head->node->type() != NodeType_VARIABLE )
    {
        edge.head->property->set_type( edge.tail->property->get_type() );
    }
}

void Graph::on_disconnect_value_side_effects( DirectedEdge edge )
{
    ASSERT( edge.tail->type_and_order() == SlotFlag_OUTPUT );

    // reset token to a default value to preserve a correct serialization
    if ( edge.head->node->type() != NodeType_VARIABLE )
    {
        Token& tok = edge.head->property->token();
        std::string buf;
        get_language()->serialize_default_buffer(buf, tok.m_type);
        tok.word_replace( buf.c_str() );
    }
}

void Graph::on_disconnect_flow_side_effects( DirectedEdge edge )
{
    ASSERT( edge.tail->type_and_order() == SlotFlag_FLOW_OUT );
    
    Scope* curr_scope = edge.head->node->parent();

    switch ( edge.head->adjacent_count())
    {
        case 0:
        {
            if ( curr_scope )
                curr_scope->remove(edge.head->node);
            break;
        }
        case 1:
        {
            Scope* adjacent_scope = edge.head->first_adjacent_node()->parent();
            if ( adjacent_scope )
                adjacent_scope->push_back(edge.head->node );
            break;
        }
        default: // 2+
        {
            // Find the lowest common ancestor of adjacent nodes
            std::vector<Scope *> adjacent_scopes;
            for (Slot *adjacent: edge.head->adjacent())
            {
                adjacent_scopes.push_back(adjacent->node->parent());
            }
            Scope *target_scope = nullptr;
            if (Scope *ancestor = Scope::lowest_common_ancestor(adjacent_scopes))
                target_scope = ancestor->get_owner()->parent();

            if (target_scope)
                target_scope->push_back(edge.head->node);
            else if ( curr_scope )
                curr_scope->remove(edge.head->node);
        }
    }
}

void Graph::on_connect_flow_side_effects( DirectedEdge edge )
{
    ASSERT( edge.tail->type_and_order() == SlotFlag_FLOW_OUT );

    Scope* target_scope       = nullptr;
    Node*  previous_node      = edge.tail->node;
    Node*  next_node          = edge.head->node;
    size_t flow_in_edge_count = edge.head->adjacent_count();

    if ( flow_in_edge_count == 1)
    {
        if ( previous_node->is_a_scope() )
        {
            Scope* inner_scope = previous_node->internal_scope();

            if ( inner_scope->child_scope().empty() )
            {
                target_scope = inner_scope;
            }
            else
            {
                target_scope = inner_scope->child_scope_at(edge.tail->position);
            }
        }
        else
        {
            target_scope = previous_node->parent();
        }
    }
    else if ( flow_in_edge_count > 1 )
    {
        // gather adjacent scopes
        std::vector<Scope*> adjacent_scope;
        for(Slot* adjacent : edge.head->adjacent() )
            adjacent_scope.push_back( adjacent->node->parent() );
        // find lowest_common_ancestor
        target_scope = Scope::lowest_common_ancestor(adjacent_scope );
    }
    else
    {
        VERIFY(false, "Unexpected edge count");
    }

    if ( target_scope )
        target_scope->push_back(next_node);
}

void Graph::disconnect( const DirectedEdge& _edge, ConnectFlags flags)
{
    // find the edge to disconnect
    SlotFlags type = _edge.tail->flags() & SlotFlag_TYPE_MASK;
    auto [range_begin, range_end]   = m_edge_registry.equal_range(type);
    auto it = std::find_if( range_begin, range_end, [&](const auto& _pair) -> bool { return _edge == _pair.second; });
    VERIFY(it != m_edge_registry.end(), "Unable to find edge" );

    // erase it from the registry
    m_edge_registry.erase(it);

    // disconnect the slots
    _edge.tail->remove_adjacent(_edge.head);
    _edge.head->remove_adjacent(_edge.tail);

    // handle side effects
    if ( flags & ConnectFlag_ALLOW_SIDE_EFFECTS )
    {
        switch ( type )
        {
            case SlotFlag_TYPE_FLOW:
            {
                on_disconnect_flow_side_effects(_edge);
                break;
            }
            case SlotFlag_TYPE_VALUE:
            {
                on_disconnect_value_side_effects(_edge);
                break;
            }
            default:
                VERIFY(!type, "Not yet implemented yet");
        }
    }

    on_change.emit();
}

IfNode* Graph::create_cond_struct()
{
    IfNode* condStructNode = m_factory->create_cond_struct();
    add(condStructNode);
    return condStructNode;
}

ForLoopNode* Graph::create_for_loop()
{
    ForLoopNode* for_loop = m_factory->create_for_loop();
    add(for_loop);
    return for_loop;
}

WhileLoopNode* Graph::create_while_loop()
{
    WhileLoopNode* while_loop = m_factory->create_while_loop();
    add(while_loop);
    return while_loop;
}

Node* Graph::create_entry_point()
{
    VERIFY( m_root.empty(), "Can't create a root child_node, already exists" );
    Node* node = m_factory->create_entry_point();
    add(node);
    m_root = node;
    return node;
}

Node* Graph::create_node()
{
    Node* node = m_factory->create_node();
    add(node);
    return node;
}

LiteralNode* Graph::create_literal(const TypeDescriptor *_type)
{
    LiteralNode* node = m_factory->create_literal(_type);
    add(node);
    return node;
}

Node* Graph::create_node( CreateNodeType _type, const FunctionDescriptor* _signature )
{
    switch ( _type )
    {
        /*
         * TODO: We could consider narowing the enum to few cases (BLOCK, VARIABLE, LITERAL, OPERATOR, FUNCTION)
         *       and rely more on _signature (ex: a bool variable could be simply "bool" or "bool bool(bool)")
         */
        case CreateNodeType_BLOCK_CONDITION:  return create_cond_struct();
        case CreateNodeType_BLOCK_FOR_LOOP:   return create_for_loop();
        case CreateNodeType_BLOCK_WHILE_LOOP: return create_while_loop();
        case CreateNodeType_BLOCK_ENTRY_POINT:clear(); return create_entry_point();

        case CreateNodeType_VARIABLE_BOOLEAN: return create_variable_decl<bool>();
        case CreateNodeType_VARIABLE_DOUBLE:  return create_variable_decl<double>();
        case CreateNodeType_VARIABLE_INTEGER: return create_variable_decl<int>();
        case CreateNodeType_VARIABLE_STRING:  return create_variable_decl<std::string>();

        case CreateNodeType_LITERAL_BOOLEAN:  return create_literal<bool>();
        case CreateNodeType_LITERAL_DOUBLE:   return create_literal<double>();
        case CreateNodeType_LITERAL_INTEGER:  return create_literal<int>();
        case CreateNodeType_LITERAL_STRING:   return create_literal<std::string>();

        case CreateNodeType_FUNCTION:
        {
            VERIFY(_signature != nullptr, "_signature is expected when dealing with functions or operators");
            Nodlang* language = get_language();
            // Currently, we handle operators and functions the exact same way
            const FunctionDescriptor* signature = language->find_function(_signature)->get_sig();
            bool is_operator = language->find_operator_fct( signature ) != nullptr;
            if ( is_operator )
                return create_operator(signature);
            return create_function(signature);
        }
        default:
            VERIFY(false, "Unhandled CreateNodeType.");
            return nullptr;
    }
}

VariableRefNode* Graph::create_variable_ref()
{
    VariableRefNode* node = m_factory->create_variable_ref();
    add(node);
    return node;
}

VariableNode* Graph::create_variable_decl(const TypeDescriptor* _type, const char*  _name)
{
    // Create variable
    VariableNode* var_node = create_variable(_type, _name );
    var_node->set_flags(VariableFlag_DECLARED); // yes, when created from the graph view, variables can be undeclared (== no scope).
    Token token(Token_t::keyword_operator, " = ");
    token.word_move_begin(1);
    token.word_move_end(-1);
    var_node->set_operator_token( token );

    return var_node;
}

Node *Graph::create_empty_instruction()
{
    Node* node = m_factory->create_empty_instruction();
    add(node);
    return node;
}

std::set<Scope *> Graph::get_root_scopes()
{
    std::set<Scope *> result;
    for(Node* node : m_node_registry)
        if ( node->is_a_scope() && !node->has_parent() )
            result.insert( node->internal_scope() );
    return result;
}

std::set<Scope *> Graph::get_scopes()
{
    std::set<Scope *> result;
    for(Node* node : m_node_registry)
        if ( node->parent() )
            result.insert( node->parent() );
    return result;
}