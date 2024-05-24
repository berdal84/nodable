#include "Graph.h"

#include <algorithm>    // std::find_if

#include "DirectedEdge.h"
#include "IfNode.h"
#include "LiteralNode.h"
#include "Node.h"
#include "NodeFactory.h"
#include "NodeUtils.h"
#include "Scope.h"
#include "VariableNode.h"
#include "language/Nodlang.h"

using namespace ndbl;
using namespace tools;

Graph::Graph(
    const NodeFactory* _factory
    )
    : m_factory(_factory)
    , m_is_dirty(false)
{
}

Graph::~Graph()
{
	clear();
}

void Graph::clear()
{
    if ( !m_root && m_node_registry.empty() && m_edge_registry.empty() )
    {
        return;
    }

	LOG_VERBOSE( "Graph", "Clearing graph ...\n")
    std::vector<PoolID<Node>> node_ids = m_node_registry; // copy to avoid iterator invalidation
    m_root.reset();
    for (auto node : node_ids)
    {
        LOG_VERBOSE("Graph", "destroying node \"%s\" (id: %zu)\n", node->name.c_str(), (u64_t)node )
        destroy(node);
    }
    m_node_registry.clear();
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

    LOG_VERBOSE("Graph", "Graph cleared.\n")
}

UpdateResult Graph::update()
{
    UpdateResult result = UpdateResult::SUCCES_WITHOUT_CHANGES;

    // Delete (flagged Nodes) / Check if dirty
    auto nodeIndex = m_node_registry.size();

    while (nodeIndex > 0)
    {
        nodeIndex--;
        PoolID<Node> node = m_node_registry.at(nodeIndex);

        if (node->flagged_to_delete)
        {
            destroy(node);
            result = UpdateResult::SUCCESS_WITH_CHANGES;
        }
        else if (node->dirty)
        {
            // TODO: dirty seems to be ignored, why do we need this? It this the remains of a refactor?
            node->dirty = false;
            result = UpdateResult::SUCCESS_WITH_CHANGES;
        }

    }

    return result;
}

void Graph::add(PoolID<Node> _node)
{
    ASSERT(std::find(m_node_registry.begin(), m_node_registry.end(), _node->poolid()) == m_node_registry.end())
	m_node_registry.push_back(_node->poolid());
    _node->parent_graph = this;
    set_dirty(); // To express this graph changed
    LOG_VERBOSE("Graph", "registerNode %s (%s)\n", _node->name.c_str(), _node->get_type()->get_name())
}

void Graph::remove(PoolID<Node> _node)
{
    auto found = std::find(m_node_registry.begin(), m_node_registry.end(), _node);
    ASSERT(found != m_node_registry.end());
    m_node_registry.erase(found);
    set_dirty(); // To express this graph changed
}

void Graph::ensure_has_root()
{
    if( is_empty() )
    {
        create_root();
    }
}

PoolID<VariableNode> Graph::create_variable(const type *_type, const std::string& _name, PoolID<Scope> _scope)
{
    PoolID<VariableNode> node = m_factory->create_variable(_type, _name, _scope);
    add(node);
	return node;
}

PoolID<Node> Graph::create_abstract_function(const func_type* _invokable, bool _is_operator)
{
    PoolID<Node> node = m_factory->create_abstract_func(_invokable, _is_operator);
    add(node);
    return node;
}

PoolID<Node> Graph::create_function(const iinvokable* _invokable, bool _is_operator)
{
    PoolID<Node> node = m_factory->create_func(_invokable, _is_operator);
    add(node);
    return node;
}

PoolID<Node> Graph::create_abstract_operator(const func_type* _invokable)
{
    return create_abstract_function(_invokable, true);
}

PoolID<Node> Graph::create_operator(const iinvokable* _invokable)
{
	return create_function(_invokable, true);
}

void Graph::destroy(PoolID<Node> _id)
{
    Node* node = _id.get();
    if( node == nullptr )
    {
        return;
    }

    // Identify each edge connected to this node
    std::vector<DirectedEdge> related_edges;
    for(auto& [_type, each_edge]: m_edge_registry)
    {
        if( each_edge.tail.node == _id || each_edge.head.node == _id )
        {
            related_edges.emplace_back(each_edge);
        }
    }
    // Disconnect all of them
    for(const DirectedEdge& each_edge : related_edges )
    {
        disconnect(each_edge);
    };

    // if it is a variable, we remove it from its scope
    if ( auto* node_variable = cast<VariableNode>(node) )
    {
        if ( IScope* scope = node_variable->get_scope().get() )
        {
            scope->remove_variable(node_variable);
        }
    }
    else if ( Scope* scope = node->get_component<Scope>().get() )
    {
        if ( !scope->has_no_variable() )
        {
            scope->remove_all_variables();
        }
    }

    // unregister and delete
    remove(_id);
    if ( m_root == _id )
    {
        m_root.reset();
    }
    m_factory->destroy_node(_id);
}

bool Graph::is_empty() const
{
    return m_root.get() == nullptr;
}

DirectedEdge* Graph::connect_or_merge(Slot&_out, Slot& _in )
{
    // Guards
    ASSERT( _in )
    ASSERT( _out )
    ASSERT( _in.has_flags( SlotFlag_INPUT ) )
    ASSERT( _in.has_flags( SlotFlag_NOT_FULL ) )
    ASSERT( _out.has_flags( SlotFlag_OUTPUT ) )
    ASSERT( _out.has_flags( SlotFlag_NOT_FULL ) )
    Property* in_prop  = _in.get_property();
    Property* out_prop = _out.get_property();
    EXPECT( in_prop, "tail property must be defined" )
    EXPECT( out_prop, "head property must be defined" )
    EXPECT( in_prop != out_prop, "Can't connect same properties!" )
    const type* out_type = out_prop->get_type();
    const type* in_type  = in_prop->get_type();
    EXPECT( type::is_implicitly_convertible( out_type, in_type ), "dependency type should be implicitly convertible to dependent type");

    // case 1: merge orphan slot
    if ( _out.get_node() == nullptr ) // if dependent is orphan
    {
        in_prop->digest( out_prop );
        delete in_prop;
        // set_dirty(); // no changes on edges/nodes
        return nullptr;
    }

    // case 2: merge non-orphan property
    if (!out_prop->is_this() && // Never a Node (property points to a node)
         _out.node->get_type()->is_child_of<LiteralNode>() && // allow to digest literals because having a node per literal is too verbose
         _in.node->get_type()->is_not_child_of<VariableNode>()) // except variables (we don't want to see the literal value in the variable node, we want the current value)
    {
        in_prop->digest( out_prop );
        destroy( _out.node);
        set_dirty(); // a node has been destroyed
        return nullptr;
    }

    // Connect (case 3)
    set_dirty();
    return connect( _out, _in, ConnectFlag_ALLOW_SIDE_EFFECTS );
}

void Graph::remove(DirectedEdge edge)
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
        LOG_WARNING("Graph", "Unable to unregister edge\n")
    }
    set_dirty(); // To express this graph changed
}

DirectedEdge* Graph::connect_to_variable(Slot& _out, VariableNode& _variable )
{
    // Guards
    ASSERT( _out.has_flags( SlotFlag_OUTPUT | SlotFlag_NOT_FULL ))
    return connect_or_merge( _out, _variable.input_slot() );
}

DirectedEdge* Graph::connect(Slot& _first, Slot& _second, ConnectFlags _flags)
{
    ASSERT( _first.has_flags( SlotFlag_ORDER_FIRST ) )
    ASSERT( _second.has_flags( SlotFlag_ORDER_SECOND ) )
    ASSERT( _first.node != _second.node )

    // Insert edge
    SlotFlags type = _first.type();

    auto& [_, edge] = *m_edge_registry.emplace( type, DirectedEdge{_first, _second});

    // Add cross-references to each end of the edge
    edge.tail->add_adjacent( edge.head );
    edge.head->add_adjacent( edge.tail );

    // Handle side effects
    if (_flags & ConnectFlag_ALLOW_SIDE_EFFECTS )
    {
        switch ( type )
        {
            case SlotFlag_TYPE_HIERARCHICAL:
            {
                // Ensure to Identify parent and child nodes
                // - parent node has a CHILD slot
                // - child node has a PARENT slot
                Node* parent    = _first.get_node();  static_assert(SlotFlag_CHILD & SlotFlag_ORDER_FIRST);
                Node* new_child = _second.get_node(); static_assert(SlotFlag_PARENT & SlotFlag_ORDER_SECOND);
                ASSERT( parent->has_component<Scope>())

                Slot* parent_next_slot    = parent->find_slot_at( SlotFlag_NEXT, _first.position() );
                ASSERT(parent_next_slot)
                Slot& new_child_prev_slot = *new_child->find_slot( SlotFlag_PREV );

                // Case 1: Parent has only 1 child (the newly connected), we connect it as "next".
                if ( !parent_next_slot->is_full() )
                {
                    connect( *parent_next_slot, new_child_prev_slot );
                }
                // Case 2: Connects to the last child's "next" slot.
                //         parent
                //           - ...
                //           - last child ->->->
                //           - new child <-<-<-
                else
                {
                    PoolID<Node> previous_child = parent->rchildren().at(1);
                    ASSERT( previous_child )

                    // Case 2.a: Connects to all last instructions' "next" slot (in last child's previous_child_scope).
                    //           parent
                    //             - ...
                    //             - previous_child
                    //                  - child 0
                    //                     - ...
                    //                     - instr n >->->->->
                    //                  - child 1
                    //                     - instr 0 >->->->->
                    //                  - ...
                    //                  - instr n ->->->->->->
                    //             - new child <-<-<-<-<-<-<-<
                    //
                    auto previous_child_scope = previous_child->get_component<Scope>().get();
                    if ( previous_child_scope && !previous_child->get_type()->is<IConditional>() )
                    {
                        for (Node* each_instr : previous_child_scope->get_last_instructions_rec() )
                        {
                            Slot* each_instr_next_slot = each_instr->find_slot( SlotFlag_NEXT | SlotFlag_NOT_FULL );
                            ASSERT(each_instr_next_slot);
                            connect( *each_instr_next_slot, new_child_prev_slot );
                        }
                    }
                    // Case 2.b: Connects to last child's "next" slot.
                    //           parent
                    //             - ...
                    //             - previous_child ->->->
                    //             - new child <-<-<-<-<-<
                    //
                    else
                    {
                        Slot* last_sibling_next_slot = previous_child->find_slot( SlotFlag_NEXT | SlotFlag_NOT_FULL );
                        ASSERT(last_sibling_next_slot)
                        connect( *last_sibling_next_slot, new_child_prev_slot );
                    }
                }
                break;
            }

            case SlotFlag_TYPE_CODEFLOW:
            {
                Node& prev_node = *_first.node; static_assert( SlotFlag_NEXT & SlotFlag_ORDER_FIRST );
                Node& next_node = *_second.node; static_assert( SlotFlag_PREV & SlotFlag_ORDER_SECOND );

                // If previous node is a scope, connects next_node as child
                if ( prev_node.has_component<Scope>() )
                {
                    connect(
                            *prev_node.find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
                            *next_node.find_slot( SlotFlag_PARENT ));
                }
                // If next node parent exists, connects next_node as a child too
                else if ( PoolID<Node> prev_parent_node = prev_node.find_parent() )
                {
                    connect(
                            *prev_parent_node->find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
                            *next_node.find_slot( SlotFlag_PARENT ));
                }

                // Connect siblings
                else if ( PoolID<Node> prev_parent_node = prev_node.find_parent() )
                {
                    Node* current_prev_node_sibling = prev_node.successors()[0].get();
                    while ( current_prev_node_sibling && current_prev_node_sibling->find_parent() )
                    {
                        connect(
                                *current_prev_node_sibling->find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
                                *prev_parent_node->find_slot( SlotFlag_PARENT ) );
                        current_prev_node_sibling = current_prev_node_sibling->successors().begin()->get();
                    }
                }
                break;
            }
            case SlotFlag_TYPE_VALUE:
            {
                // Clear in_token and transfer out_token prefix/suffix/type
                //
                //   <prefix> dependent <suffix>    (output)
                //       |       out        |
                //       |        |         |
                //       |        |         |
                //       v       in         v
                //    < ... > dependency < ... >    (input)
                //
                Token& out_token = _first.get_property()->token;  static_assert(SlotFlag_OUTPUT & SlotFlag_ORDER_FIRST);
                Token& in_token  = _second.get_property()->token; static_assert(SlotFlag_INPUT & SlotFlag_ORDER_SECOND);

                if ( out_token.is_null() || in_token.is_null() )
                {
                    break;
                }

                in_token.clear();
                in_token.m_type = out_token.m_type;
                in_token.move_prefixsuffix( &out_token );
                break;
            }
            default:
                ASSERT(false);// This connection type is not yet implemented
        }
    }
    set_dirty(); // To express this graph changed
    return &edge;
}

void Graph::disconnect( const DirectedEdge& _edge, ConnectFlags flags)
{
    // find the edge to disconnect
    SlotFlags type = _edge.tail.flags & SlotFlag_TYPE_MASK;
    auto [range_begin, range_end]   = m_edge_registry.equal_range(type);
    auto it = std::find_if( range_begin, range_end, [&](const auto& _pair) -> bool { return _edge == _pair.second; });
    EXPECT( it != m_edge_registry.end(), "Unable to find edge" );

    // erase it from the registry
    m_edge_registry.erase(it);

    // update tail/head slots accordingly
    _edge.tail->remove_adjacent( _edge.head );
    _edge.head->remove_adjacent( _edge.tail );

    // disconnect effectively
    switch ( type )
    {
        case SlotFlag_TYPE_CODEFLOW:
        {
            Node* successor = _edge.tail.node.get();
            Node* successor_parent = successor->find_parent().get();
            if ( flags & ConnectFlag_ALLOW_SIDE_EFFECTS && successor_parent  )
            {
                while (successor && successor_parent->poolid() == successor->find_parent() )
                {
                    disconnect({ *successor->find_slot( SlotFlag_PARENT ), *successor->find_parent()->find_slot( SlotFlag_CHILD ) } );
                    successor = successor->successors().begin()->get();
                }
            }

            break;
        }

        case SlotFlag_TYPE_HIERARCHICAL:
        case SlotFlag_TYPE_VALUE:
            // None
            break;
        default:
            EXPECT(!type, "Not yet implemented yet");
    }

    set_dirty(); // To express this graph changed
}

PoolID<Node> Graph::create_scope()
{
    PoolID<Node> scopeNode = m_factory->create_scope();
    add(scopeNode);
    return scopeNode;
}

PoolID<IfNode> Graph::create_cond_struct()
{
    PoolID<IfNode> condStructNode = m_factory->create_cond_struct();
    add(condStructNode);
    return condStructNode;
}

PoolID<ForLoopNode> Graph::create_for_loop()
{
    PoolID<ForLoopNode> for_loop = m_factory->create_for_loop();
    add(for_loop);
    return for_loop;
}

PoolID<WhileLoopNode> Graph::create_while_loop()
{
    PoolID<WhileLoopNode> while_loop = m_factory->create_while_loop();
    add(while_loop);
    return while_loop;
}

PoolID<Node> Graph::create_root()
{
    PoolID<Node> node = m_factory->create_program();
    add(node);
    m_root = node;
    return node;
}

PoolID<Node> Graph::create_node()
{
    PoolID<Node> node = m_factory->create_node();
    add(node);
    return node;
}

PoolID<LiteralNode> Graph::create_literal(const type *_type)
{
    PoolID<LiteralNode> node = m_factory->create_literal(_type);
    add(node);
    return node;
}

PoolID<Node> Graph::create_node( NodeType _type, const func_type* _signature )
{
    switch ( _type )
    {
        /*
         * TODO: We could consider narowing the enum to few cases (BLOCK, VARIABLE, LITERAL, OPERATOR, FUNCTION)
         *       and rely more on _signature (ex: a bool variable could be simply "bool" or "bool bool(bool)")
         */
        case NodeType_BLOCK_CONDITION:  return create_cond_struct();
        case NodeType_BLOCK_FOR_LOOP:   return create_for_loop();
        case NodeType_BLOCK_WHILE_LOOP: return create_while_loop();
        case NodeType_BLOCK_SCOPE:      return create_scope();
        case NodeType_BLOCK_PROGRAM:    clear(); return create_root();

        case NodeType_VARIABLE_BOOLEAN: return create_variable_decl<bool>();
        case NodeType_VARIABLE_DOUBLE:  return create_variable_decl<double>();
        case NodeType_VARIABLE_INTEGER: return create_variable_decl<int>();
        case NodeType_VARIABLE_STRING:  return create_variable_decl<std::string>();

        case NodeType_LITERAL_BOOLEAN:  return create_literal<bool>();
        case NodeType_LITERAL_DOUBLE:   return create_literal<double>();
        case NodeType_LITERAL_INTEGER:  return create_literal<int>();
        case NodeType_LITERAL_STRING:   return create_literal<std::string>();

        case NodeType_INVOKABLE:
        {
            EXPECT(_signature != nullptr, "_signature is expected when dealing with functions or operators")
            Nodlang* language = get_language();
            // Currently, we handle operators and functions the exact same way
            const auto invokable = language->find_function(_signature);
            bool is_operator = language->find_operator_fct( invokable->get_type() ) != nullptr;
            return create_function(invokable.get(), is_operator);
        }
        default:
            EXPECT( false, "Unhandled NodeType.");
    }
}

PoolID<VariableNode> Graph::create_variable_decl(const type* _type, const char*  _name, PoolID<Scope>  _scope)
{
    if( !_scope)
    {
        _scope = get_root()->get_component<Scope>();
    }

    // Create variable
    PoolID<VariableNode> var_node = create_variable(_type, _name, _scope );
    var_node->set_declared(true);
    Token token(Token_t::keyword_operator, " = ");
    token.m_word_start_pos = 1;
    token.m_word_size = 1;
    var_node->assignment_operator_token = token;

    // TODO: attach a default Literal?

    return var_node;
}
