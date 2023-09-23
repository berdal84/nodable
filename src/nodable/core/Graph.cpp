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

DirectedEdge Graph::connect_or_digest(Slot* dependent, Slot* dependency )
{
    if( dependent->flags & SlotFlag_ACCEPTS_DEPENDENTS )
    {
       std::swap(dependent, dependency);
    }
    FW_EXPECT( dependent->flags == SlotFlag_INPUT,  "tail slot must be a dependent")
    FW_EXPECT( dependency->flags == SlotFlag_OUTPUT, "head slot must be a dependency")

    Property* dependent_prop  = dependent->get_property();
    Property* dependency_prop = dependency->get_property();

    FW_EXPECT( dependent_prop, "tail property must be defined" )
    FW_EXPECT( dependency_prop, "head property must be defined" )
    FW_EXPECT( dependent_prop != dependency_prop, "Can't connect same properties!" )

    const fw::type* dependency_type = dependency_prop->get_type();
    const fw::type* dependent_type  = dependent_prop->get_type();

    FW_EXPECT( fw::type::is_implicitly_convertible( dependency_type, dependent_type), "dependency type should be implicitly convertible to dependent type");

    /*
     * If _from has no owner _to can digest it, no need to create an edge in this case.
     */
    if ( dependent->get_node() == nullptr )
    {
       dependency_prop->digest( dependent_prop );
        delete dependent_prop;
        return {};
    }

    if (
        !dependent_prop->is_this() &&
        dependent->node->get_type()->is_child_of<LiteralNode>() &&
        dependency->node->get_type()->is_not_child_of<VariableNode>())
    {
        dependency_prop->digest( dependent_prop );
        destroy( dependent->node);
        set_dirty();
        return DirectedEdge::null;
    }

    DirectedEdge edge = connect( dependent, dependency, SideEffects::ON );

    // TODO: move this somewhere else
    // (transfer prefix/suffix)
    Token* src_token = &dependent_prop->token;
    if (!src_token->is_null())
    {
        if (!dependency_prop->token.is_null())
        {
            dependency_prop->token.clear();
            dependency_prop->token.m_type = src_token->m_type;
        }
        dependency_prop->token.transfer_prefix_and_suffix_from(src_token);
    }
    set_dirty();
    return edge;
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

DirectedEdge Graph::connect_to_instruction(Slot* expression_root, InstructionNode* instruction )
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
    return connect_or_digest(
            expression_node->find_slot( THIS_PROPERTY, SlotFlag_OUTPUT ),
            &instruction->root_slot() );
}

DirectedEdge Graph::connect_to_variable(Slot* tail, PoolID<VariableNode> variable_id )
{
    FW_EXPECT( tail->flags == SlotFlag_OUTPUT, "Tail should be an OUT VALUE" )
    return connect_or_digest(tail, variable_id->find_value_typed_slot( SlotFlag_INPUT ) );
}

DirectedEdge Graph::connect(Slot* _tail, Slot* _head, SideEffects _flags)
{
    DirectedEdge edge{*_tail, *_head};
    DirectedEdge::normalize(edge);

    Node* dependent_node = edge.tail.node.get();
    Node* dependency_node = edge.head.node.get();
    SlotFlags type = edge.tail.flags & SlotFlag_TYPE_MASK;

    if (_flags == SideEffects::ON )
    {
        switch ( type )
        {
            case SlotFlag_TYPE_HIERARCHICAL:
            {
                FW_ASSERT( dependency_node->has_component<Scope>())
                Slot* dependent_previous_slot = dependent_node->find_slot( SlotFlag_PREV );

                Slot* next_slot = dependency_node->find_slot( SlotFlag_NEXT );
                if ( next_slot && !next_slot->is_full() )
                {
                    connect(
                        dependent_previous_slot,
                        dependency_node->find_slot( SlotFlag_NEXT ),
                        SideEffects::OFF );
                }
                else if (Node* dependency_last_child = dependency_node->last_child() )
                {
                    if (auto scope = dependency_last_child->get_component<Scope>().get())
                    {
                        std::vector<InstructionNode *> last_instructions = scope->get_last_instructions_rec();
                        if (!last_instructions.empty())
                        {
                            LOG_VERBOSE("Graph", "Empty scope found when trying to connect(...)");
                        }
                        for (InstructionNode *each_instruction: last_instructions )
                        {
                            connect(
                                dependent_previous_slot,
                                each_instruction->find_slot( SlotFlag_NEXT ),
                                SideEffects::OFF );
                        }
                    }
                    else
                    {
                        Slot* dependency_last_child_next_slot = dependency_last_child->find_slot( SlotFlag_NEXT );
                        FW_ASSERT(!dependency_last_child_next_slot->is_full())
                        connect( dependent_previous_slot, dependency_last_child_next_slot, SideEffects::OFF);
                    }
                }
                break;
            }

            case SlotFlag_TYPE_CODEFLOW:

                if ( dependency_node->has_component<Scope>())
                {
                    connect(
                        dependent_node->find_slot( SlotFlag_PARENT ),
                        dependency_node->find_slot( SlotFlag_CHILD ),
                        SideEffects::OFF );
                }
                else if (Node *dst_parent = dependency_node->get_parent().get())
                {
                    connect(
                        dependent_node->find_slot( SlotFlag_PARENT ),
                        dst_parent->find_slot( SlotFlag_CHILD ),
                        SideEffects::OFF );
                }

                /**
                 * create child/get_parent() link with dst_parent
                 */
                if (Node *src_parent = dependent_node->get_parent().get())
                {
                    Node* current_successor = dependent_node->successors().begin()->get();
                    while (current_successor && current_successor->get_parent().get() != nullptr)
                    {
                        connect(
                            current_successor->find_slot( SlotFlag_PARENT ),
                            src_parent->find_slot( SlotFlag_CHILD ),
                            SideEffects::OFF );
                        current_successor = current_successor->successors().begin()->get();
                    }
                }
                break;


            case SlotFlag_TYPE_VALUE:
                // no side effect
                break;

            default:
                FW_ASSERT(false);// This connection type is not yet implemented
        }
    }

    edge.tail->add_adjacent( edge.head );
    edge.head->add_adjacent( edge.tail );

    m_edge_registry.emplace( type, edge);
    set_dirty();
    return edge;
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
