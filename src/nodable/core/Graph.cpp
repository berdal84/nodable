#include "Graph.h"

#include <algorithm>    // std::find_if

#include "ConditionalStructNode.h"
#include "DirectedEdge.h"
#include "InstructionNode.h"
#include "LiteralNode.h"
#include "Node.h"
#include "NodeFactory.h"
#include "NodeUtils.h"
#include "Scope.h"
#include "VariableNode.h"
#include "language/Nodlang.h"

using namespace ndbl;

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
        LOG_VERBOSE("Graph", "destroying node \"%s\" (id: %zu)\n", node->name.c_str(), (u32_t)node )
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

    set_dirty(false);

    return result;
}

void Graph::add(PoolID<Node> _node)
{
    FW_ASSERT(std::find(m_node_registry.begin(), m_node_registry.end(), _node->poolid()) == m_node_registry.end())
	m_node_registry.push_back(_node->poolid());
    _node->parent_graph = this;
    LOG_VERBOSE("Graph", "registerNode %s (%s)\n", _node->name.c_str(), _node->get_type()->get_name())
}

void Graph::remove(PoolID<Node> _node)
{
    auto found = std::find(m_node_registry.begin(), m_node_registry.end(), _node);
    FW_ASSERT(found != m_node_registry.end());
    m_node_registry.erase(found);
}

PoolID<InstructionNode> Graph::create_instr()
{
    PoolID<InstructionNode> instructionNode = m_factory->create_instr();
    add(instructionNode);

	return instructionNode;
}

void Graph::ensure_has_root()
{
    if( is_empty() )
    {
        create_root();
    }
}

PoolID<VariableNode> Graph::create_variable(const fw::type *_type, const std::string& _name, PoolID<Scope> _scope)
{
    PoolID<VariableNode> node = m_factory->create_variable(_type, _name, _scope);
    add(node);
	return node;
}

PoolID<Node> Graph::create_abstract_function(const fw::func_type* _invokable, bool _is_operator)
{
    PoolID<Node> node = m_factory->create_abstract_func(_invokable, _is_operator);
    add(node);
    return node;
}

PoolID<Node> Graph::create_function(const fw::iinvokable* _invokable, bool _is_operator)
{
    PoolID<Node> node = m_factory->create_func(_invokable, _is_operator);
    add(node);
    return node;
}

PoolID<Node> Graph::create_abstract_operator(const fw::func_type* _invokable)
{
    return create_abstract_function(_invokable, true);
}

PoolID<Node> Graph::create_operator(const fw::iinvokable* _invokable)
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
        disconnect(each_edge, SideEffects::OFF );
    };

    // if it is a variable, we remove it from its scope
    if ( auto* node_variable = fw::cast<VariableNode>(node) )
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

DirectedEdge* Graph::connect_or_merge(Slot* _out, Slot*_in )
{
    Property* in_prop  = _in->get_property();
    Property* out_prop = _out->get_property();

    // Guards
    FW_EXPECT( _in->flags == SlotFlag_INPUT,  "tail slot must be a dependent")
    FW_EXPECT( _out->flags == SlotFlag_OUTPUT, "head slot must be a dependency")
    FW_EXPECT( in_prop, "tail property must be defined" )
    FW_EXPECT( out_prop, "head property must be defined" )
    FW_EXPECT( in_prop != out_prop, "Can't connect same properties!" )
    const fw::type* out_type = out_prop->get_type();
    const fw::type* in_type  = in_prop->get_type();
    FW_EXPECT( fw::type::is_implicitly_convertible( out_type, in_type ), "dependency type should be implicitly convertible to dependent type");

    // case 1: merge orphan slot
    if ( _in->get_node() == nullptr ) // if dependent is orphan
    {
        out_prop->digest( in_prop );
        delete in_prop;
        // set_dirty(); // no changes on edges/nodes
        return nullptr;
    }

    // case 2: merge non-orphan property
    if (!in_prop->is_this() && // Never digest a Node (property points to a node)
         _out->node->get_type()->is_child_of<LiteralNode>() && // allow to digest literals because having a node per literal is too verbose
         _in->node->get_type()->is_not_child_of<VariableNode>()) // except variables (we don't want to see the literal value in the variable node, we want the current value)
    {
        in_prop->digest( out_prop );
        destroy( _out->node);
        set_dirty(); // a node has been destroyed
        return nullptr;
    }

    // Connect (case 3)
    set_dirty();
    return connect( _out, _in, SideEffects::ON );
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
}

DirectedEdge* Graph::connect_to_instruction(Slot* expression_root, InstructionNode* instruction )
{
    Node* expression_node = expression_root->get_node();

    if ( auto* variable = fw::cast<VariableNode>( expression_node ) )
    {
        // Define variable's declaration instruction ONCE.
        // TODO: reconsider this, user should be able to change this  dynamically.
        if( variable->get_declaration_instr() == PoolID<InstructionNode>::null )
        {
            variable->set_declaration_instr( instruction->poolid());
        }
    }
    return connect_or_merge(
            expression_node->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
            &instruction->root_slot() );
}

DirectedEdge* Graph::connect_to_variable(Slot* _out, PoolID<VariableNode> _variable_in )
{
    FW_EXPECT( _out->flags == SlotFlag_OUTPUT, "slot should be an OUTPUT" )
    return connect_or_merge( _out, _variable_in->find_value_typed_slot( SlotFlag_INPUT ) );
}

DirectedEdge* Graph::connect(Slot* _out, Slot* _in, SideEffects _flags)
{
    FW_ASSERT( _out->flags & SlotFlag_ORDER_PRIMARY )
    FW_ASSERT( _in->flags & SlotFlag_ORDER_SECONDARY )

    // Insert edge
    SlotFlags type = _out->type();
    auto& [_, edge] = *m_edge_registry.emplace( type, DirectedEdge{*_out, *_in });

    // Add cross-references to each end of the edge
    edge.tail->add_adjacent( edge.head );
    edge.head->add_adjacent( edge.tail );

    // Handle side effects
    if (_flags == SideEffects::ON )
    {
        Node* out_node = _out->get_node();
        Node* in_node = _in->get_node();

        switch ( type )
        {
            case SlotFlag_TYPE_HIERARCHICAL:
            {
                FW_ASSERT( in_node->has_component<Scope>())
                Slot* out_previous_slot = out_node->find_slot( SlotFlag_PREV );
                Slot* in_next_slot      = in_node->find_slot( SlotFlag_NEXT );
                if ( in_next_slot && !in_next_slot->is_full() )
                {
                    connect(
                            out_previous_slot,
                            in_next_slot,
                            SideEffects::OFF );
                }
                else if (Node* last_child = in_node->last_child() )
                {
                    if (auto scope = last_child->get_component<Scope>().get())
                    {
                        std::vector<InstructionNode *> last_instructions = scope->get_last_instructions_rec();
                        if (!last_instructions.empty())
                        {
                            LOG_VERBOSE("Graph", "Empty scope found when trying to connect(...)");
                        }
                        for (InstructionNode* each_instruction: last_instructions )
                        {
                            connect(
                                out_previous_slot,
                                each_instruction->find_slot( SlotFlag_NEXT ),
                                SideEffects::OFF );
                        }
                    }
                    else
                    {
                        Slot*last_child_next_slot = last_child->find_slot( SlotFlag_NEXT );
                        FW_ASSERT(!last_child_next_slot->is_full())
                        connect( out_previous_slot, last_child_next_slot, SideEffects::OFF);
                    }
                }
                break;
            }

            case SlotFlag_TYPE_CODEFLOW:

                if ( in_node->has_component<Scope>())
                {
                    connect(
                        out_node->find_slot( SlotFlag_PARENT ),
                        in_node->find_slot( SlotFlag_CHILD ),
                        SideEffects::OFF );
                }
                else if (Node* dependency_parent_node = in_node->get_parent().get())
                {
                    connect(
                        out_node->find_slot( SlotFlag_PARENT ),
                        dependency_parent_node->find_slot( SlotFlag_CHILD ),
                        SideEffects::OFF );
                }

                /**
                 * create child/get_parent() link with dst_parent
                 */
                if (Node* out_parent_node = out_node->get_parent().get())
                {
                    Node* current_successor = out_node->successors().begin()->get();
                    while (current_successor && current_successor->get_parent().get() != nullptr)
                    {
                        connect(
                            current_successor->find_slot( SlotFlag_PARENT ),
                            out_parent_node->find_slot( SlotFlag_CHILD ),
                            SideEffects::OFF );
                        current_successor = current_successor->successors().begin()->get();
                    }
                }
                break;


            case SlotFlag_TYPE_VALUE:
            {
                // Transfer token prefix/suffix/type
                //
                //   <prefix> dependent <suffix>    (output)
                //       |        ^         |
                //       v        |         v
                //    < ... > dependency < ... >    (input)
                //
                Token &dependent_token  = edge.tail->get_property()->token;
                Token &dependency_token = edge.head->get_property()->token;
                if ( dependent_token.is_null() || dependency_token.is_null() )
                {
                    break;
                }
                dependency_token.clear();
                dependency_token.m_type = dependent_token.m_type;
                dependency_token.move_prefixsuffix( &dependent_token );
                break;
            }
            default:
                FW_ASSERT(false);// This connection type is not yet implemented
        }
    }
    set_dirty();
    return &edge;
}

void Graph::disconnect(DirectedEdge _edge, SideEffects flags)
{
    // find the edge to disconnect
    SlotFlags type = _edge.tail.flags & SlotFlag_TYPE_MASK;
    auto [range_begin, range_end]   = m_edge_registry.equal_range(type);
    auto it = std::find_if( range_begin, range_end, [&](const auto& _pair) -> bool { return _edge == _pair.second; });
    FW_EXPECT( it != m_edge_registry.end(), "Unable to find edge" );

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
            Node* successor_parent = successor->get_parent().get();
            if ( flags == SideEffects::ON && successor_parent  )
            {
                while (successor && successor_parent->poolid() == successor->get_parent() )
                {
                    disconnect({ *successor->find_slot( SlotFlag_PARENT ), *successor->get_parent()->find_slot( SlotFlag_CHILD ) }, SideEffects::OFF );
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
            FW_EXPECT(!type, "Not yet implemented yet");
    }

    set_dirty();
}

PoolID<Node> Graph::create_scope()
{
    PoolID<Node> scopeNode = m_factory->create_scope();
    add(scopeNode);
    return scopeNode;
}

PoolID<ConditionalStructNode> Graph::create_cond_struct()
{
    PoolID<ConditionalStructNode> condStructNode = m_factory->create_cond_struct();
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

PoolID<LiteralNode> Graph::create_literal(const fw::type *_type)
{
    PoolID<LiteralNode> node = m_factory->create_literal(_type);
    add(node);
    return node;
}
