#include "Graph.h"

#include <algorithm>    // std::find_if

#include "ConditionalStructNode.h"
#include "InstructionNode.h"
#include "LiteralNode.h"
#include "Node.h"
#include "NodeFactory.h"
#include "NodeUtils.h"
#include "Scope.h"
#include "TDirectedEdge.h"
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
	LOG_VERBOSE( "Graph", "Clearing graph ...\n")

	if ( !m_node_registry.empty() )
	{
        std::vector<ID<Node>> node_ids = m_node_registry; // copy to avoid iterator invalidation
        for (auto node : node_ids)
        {
            LOG_VERBOSE("Graph", "remove and delete: %s \n", node->name.c_str() )
            destroy(node);
        }
	}
	else
    {
        LOG_VERBOSE("Graph", "No nodes in registry.\n")
    }
    m_node_registry.clear();
    FW_EXPECT(m_edge_registry.empty(), "m_edge_registry should be empty because all nodes have been deleted.");
    m_root.reset();

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
        ID<Node> node = m_node_registry.at(nodeIndex);

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

void Graph::add(ID<Node> _node)
{
    FW_ASSERT(std::find(m_node_registry.begin(), m_node_registry.end(), _node->id()) == m_node_registry.end())
	m_node_registry.push_back(_node->id());
    _node->parent_graph = this;
    LOG_VERBOSE("Graph", "registerNode %s (%s)\n", _node->name.c_str(), _node->get_type()->get_name())
}

void Graph::remove(ID<Node> _node)
{
    auto found = std::find(m_node_registry.begin(), m_node_registry.end(), _node);
    m_node_registry.erase(found);
}

ID<InstructionNode> Graph::create_instr()
{
    ID<InstructionNode> instructionNode = m_factory->create_instr();
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

ID<VariableNode> Graph::create_variable(const fw::type *_type, const std::string& _name, ID<Scope> _scope)
{
    ID<VariableNode> node = m_factory->create_variable(_type, _name, _scope);
    add(node);
	return node;
}

ID<Node> Graph::create_abstract_function(const fw::func_type* _invokable, bool _is_operator)
{
    ID<Node> node = m_factory->create_abstract_func(_invokable, _is_operator);
    add(node);
    return node;
}

ID<Node> Graph::create_function(const fw::iinvokable* _invokable, bool _is_operator)
{
    ID<Node> node = m_factory->create_func(_invokable, _is_operator);
    add(node);
    return node;
}

ID<Node> Graph::create_abstract_operator(const fw::func_type* _invokable)
{
    return create_abstract_function(_invokable, true);
}

ID<Node> Graph::create_operator(const fw::iinvokable* _invokable)
{
	return create_function(_invokable, true);
}

void Graph::destroy(ID<Node> _node)
{
    if( _node.get() == nullptr )
    {
        return;
    }

    // disconnect any edge connected to this node
    for(const Edge& each_edge : _node->edges())
    {
        disconnect(each_edge, ConnectFlag::SIDE_EFFECTS_OFF );
    };

    // if it is a variable, we remove it from its scope
    if ( VariableNode* node_variable = fw::cast<VariableNode> (_node.get() ) )
    {
        if ( IScope* scope = node_variable->get_scope().get() )
        {
            scope->remove_variable(node_variable);
        }
    }
    else if ( Scope* scope = _node->get_component<Scope>().get() )
    {
        if ( !scope->has_no_variable() )
        {
            scope->remove_all_variables();
        }
    }

    // unregister and delete
    remove(_node->id());
    if ( m_root == _node->id() )
    {
        m_root.reset();
    }
    m_factory->destroy_node(_node);
}

bool Graph::is_empty() const
{
    return m_root.get() == nullptr;
}

Edge Graph::connect(Slot tail, Slot head)
{
    Property* tail_property = tail.get_property();
    Property* head_property = head.get_property();

    FW_EXPECT( tail_property != head_property, "Can't connect same properties!" )
    FW_EXPECT( fw::type::is_implicitly_convertible(tail_property->get_type(), head_property->get_type()), "Can't connect non implicitly convertible Properties!");

    /*
     * If _from has no owner _to can digest it, no need to create an edge in this case.
     */
    if ( tail.get_node() == nullptr )
    {
        head_property->digest( tail_property );
        delete tail_property;
        return {};
    }

    if (
        !tail_property->is_referencing<Node>() &&
        tail.node->get_type()->is_child_of<LiteralNode>() &&
        head.node->get_type()->is_not_child_of<VariableNode>())
    {
        head_property->digest( tail_property );
        destroy(tail.node);
        set_dirty();
        return Edge::null;
    }

    Edge edge = connect(tail, Relation::WRITE_READ, head, ConnectFlag::SIDE_EFFECTS_ON);

    // TODO: move this somewhere else
    // (transfer prefix/suffix)
    Token* src_token = &tail_property->token;
    if (!src_token->is_null())
    {
        if (!head_property->token.is_null())
        {
            head_property->token.clear();
            head_property->token.m_type = src_token->m_type;
        }
        head_property->token.transfer_prefix_and_suffix_from(src_token);
    }
    set_dirty();
    return edge;
}

void Graph::remove(Edge edge)
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

Edge Graph::connect(Node* tail_node, InstructionNode* head_node)
{
    // set declaration_instr once
    if ( auto* variable = fw::cast<VariableNode>(tail_node) )
    {
        if( variable->get_declaration_instr().get() == nullptr )
        {
            variable->set_declaration_instr(head_node->id());
        }
    }
    return connect(tail_node->slot(Way::Out), head_node->root_slot() );
}

Edge Graph::connect(Slot tail, VariableNode* head_node)
{
    return connect(tail, head_node->get_value_slot( Way::In ) );
}

Edge Graph::connect(Slot _tail, Relation _type, Slot _head, ConnectFlag _flags)
{
    Edge edge{_tail, _type, _head};
    sanitize_edge(edge);

    Node *tail_node = edge.tail.node.get();
    Node *head_node = edge.head.node.get();

    if (_flags == ConnectFlag::SIDE_EFFECTS_ON)
    {
        switch ( edge.relation )
        {
            case Relation::CHILD_PARENT:
            {
                FW_ASSERT(head_node->has_component<Scope>())

                if (head_node->allows_more(Relation::NEXT_PREVIOUS))// directly
                {
                    connect(tail_node->slot(), Relation::NEXT_PREVIOUS, head_node->slot(), ConnectFlag::SIDE_EFFECTS_OFF);
                } else if (Node *tail = head_node->children().back().get())// to the last children
                {
                    if (auto scope = tail->get_component<Scope>().get())
                    {
                        std::vector<InstructionNode *> tails;
                        scope->get_last_instructions_rec(tails);

                        for (InstructionNode *each_instruction: tails)
                        {
                            connect(tail_node->slot(), Relation::NEXT_PREVIOUS, each_instruction->slot(), ConnectFlag::SIDE_EFFECTS_OFF);
                        }

                        if (!tails.empty()) LOG_VERBOSE("Graph", "Empty scope found when trying to connect(...)");
                    }
                    else if ( tail->allows_more(Relation::NEXT_PREVIOUS) )
                    {
                        connect(tail_node->slot(), Relation::NEXT_PREVIOUS, tail->slot(), ConnectFlag::SIDE_EFFECTS_OFF);
                    }
                }
                break;
            }

            case Relation::WRITE_READ:
                // no side effect
                break;

            case Relation::NEXT_PREVIOUS:

                if (head_node->has_component<Scope>())
                {
                    connect(tail_node->slot(), Relation::CHILD_PARENT, head_node->slot(), ConnectFlag::SIDE_EFFECTS_OFF);
                } else if (Node *dst_parent = head_node->parent.get())
                {
                    connect(tail_node->slot(), Relation::CHILD_PARENT, dst_parent->slot(), ConnectFlag::SIDE_EFFECTS_OFF);
                }

                /**
                 * create child/parent link with dst_parent
                 */
                if (Node *src_parent = tail_node->parent.get())
                {
                    Node* current_successor = tail_node->successors().begin()->get();
                    while (current_successor && current_successor->parent.get() != nullptr)
                    {
                        connect(current_successor->slot(), Relation::CHILD_PARENT, src_parent->slot(), ConnectFlag::SIDE_EFFECTS_OFF);
                        current_successor = current_successor->successors().begin()->get();
                    }
                }
                break;

            default:
                FW_ASSERT(false);// This connection type is not yet implemented
        }
    }
    tail_node->add_edge(edge);
    head_node->add_edge(edge);

    m_edge_registry.emplace(edge.relation, edge);
    set_dirty();
    return edge;
}

void Graph::disconnect(Edge _edge, ConnectFlag flags)
{
    // find the edge to disconnect
    auto [begin, end] = m_edge_registry.equal_range(_edge.relation );
    auto found = std::find_if(begin, end, [&_edge](auto& pair)
    {
        return pair.second == _edge;
    });

    if(found == end) return;

    Node* tail_node = _edge.tail.get_node();
    Node* head_node = _edge.head.get_node();

    // disconnect effectively
    switch (_edge.relation )
    {
        case Relation::CHILD_PARENT:
            tail_node->set_parent({});
            break;

        case Relation::WRITE_READ:
            tail_node->remove_edge(_edge);
            head_node->remove_edge(_edge);
            break;

        case Relation::NEXT_PREVIOUS:
        {
            Node* successor = tail_node;
            Node* successor_parent = successor->parent.get();
            if ( flags == ConnectFlag::SIDE_EFFECTS_ON && successor_parent  )
            {
                while (successor && successor->parent == tail_node->parent )
                {
                    Edge child_of_edge{successor->slot(), Relation::CHILD_PARENT, tail_node->parent->slot()};
                    disconnect( child_of_edge, ConnectFlag::SIDE_EFFECTS_OFF );
                    successor = successor->successors().begin()->get();
                }
            }

            break;
        }
        default:
            FW_ASSERT(false); // This connection type is not yet implemented
    }

   set_dirty();
}

ID<Node> Graph::create_scope()
{
    ID<Node> scopeNode = m_factory->create_scope();
    add(scopeNode);
    return scopeNode;
}

ID<ConditionalStructNode> Graph::create_cond_struct()
{
    ID<ConditionalStructNode> condStructNode = m_factory->create_cond_struct();
    add(condStructNode);
    return condStructNode;
}

ID<ForLoopNode> Graph::create_for_loop()
{
    ID<ForLoopNode> for_loop = m_factory->create_for_loop();
    add(for_loop);
    return for_loop;
}

ID<WhileLoopNode> Graph::create_while_loop()
{
    ID<WhileLoopNode> while_loop = m_factory->create_while_loop();
    add(while_loop);
    return while_loop;
}

ID<Node> Graph::create_root()
{
    ID<Node> node = m_factory->create_program();
    add(node);
    m_root = node;
    return node;
}

ID<Node> Graph::create_node()
{
    ID<Node> node = m_factory->create_node();
    add(node);
    return node;
}

ID<LiteralNode> Graph::create_literal(const fw::type *_type)
{
    ID<LiteralNode> node = m_factory->create_literal(_type);
    add(node);
    return node;
}

Edge Graph::connect(Edge edge, ConnectFlag flags)
{
    return connect(edge.tail, edge.relation, edge.head, flags);
}
