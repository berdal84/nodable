#include <ndbl/core/GraphNode.h>

#include <algorithm>    // std::find_if

#include "fw/core/assertions.h"
#include "fw/core/log.h"

#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/DirectedEdge.h>
#include <ndbl/core/INodeFactory.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/LiteralNode.h>
#include <ndbl/core/Node.h>
#include <ndbl/core/Scope.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/language/Nodlang.h>

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<GraphNode>("GraphNode").extends<Node>();
}

GraphNode::GraphNode(const Nodlang* _language, const INodeFactory* _factory, const bool* _autocompletion)
    : m_language(_language)
    , m_factory(_factory)
    , m_root(nullptr)
    , m_autocompletion(_autocompletion)
{
}

GraphNode::~GraphNode()
{
	clear();
}

void GraphNode::clear()
{
	LOG_VERBOSE( "GraphNode", "Clearing graph ...\n")

	if ( !m_node_registry.empty() )
	{
        for (auto i = m_node_registry.size(); i > 0; i--)
        {
            Node* node = m_node_registry[i - 1];
            LOG_VERBOSE("GraphNode", "remove and delete: %s \n", node->get_name() )
            destroy(node);
        }
	}
	else
    {
        LOG_VERBOSE("GraphNode", "No nodes in registry.\n")
    }
    m_node_registry.clear();
    FW_EXPECT(m_edge_registry.empty(), "m_edge_registry should be empty because all nodes have been deleted.");
    m_root = nullptr;

    LOG_VERBOSE("GraphNode", "Graph cleared.\n")
}

UpdateResult GraphNode::update()
{
    UpdateResult result = UpdateResult::Success_NoChanges;

    // Delete (flagged Nodes) / Check if dirty
    {
        auto nodeIndex = m_node_registry.size();

        while (nodeIndex > 0)
        {
            nodeIndex--;
            auto node = m_node_registry.at(nodeIndex);

            if (node->flagged_to_delete())
            {
                destroy(node);
                result = UpdateResult::Success_WithChanges;
            }
            else if (node->is_dirty()) // We just check if the node is dirty
            {
                node->set_dirty(false);
                result = UpdateResult::Success_WithChanges;
            }

        }
    }

    set_dirty(false);

    return result;
}

void GraphNode::add(Node* _node)
{
	m_node_registry.push_back(_node);
    _node->set_parent_graph(this);
    LOG_VERBOSE("GraphNode", "registerNode %s (%s)\n", _node->get_name(), _node->get_type().get_name())
}

void GraphNode::remove(Node* _node)
{
    auto found = std::find(m_node_registry.begin(), m_node_registry.end(), _node);
    m_node_registry.erase(found);
}

InstructionNode* GraphNode::create_instr()
{
	auto instructionNode = m_factory->new_instr();
    add(instructionNode);

	return instructionNode;
}

void GraphNode::ensure_has_root()
{
    if( is_empty() )
    {
        create_root();
    }
}

VariableNode* GraphNode::create_variable(fw::type _type, const std::string& _name, IScope* _scope)
{
    auto node = m_factory->new_variable(_type, _name, _scope);
    add(node);
	return node;
}

Node* GraphNode::create_abstract_function(const fw::func_type* _invokable, bool _is_operator)
{
    Node* node = m_factory->new_abstract_function(_invokable, _is_operator);
    add(node);
    return node;
}

Node* GraphNode::create_function(const fw::iinvokable* _invokable, bool _is_operator)
{
    Node* node = m_factory->new_function(_invokable, _is_operator);
    add(node);
    return node;
}

Node* GraphNode::create_abstract_operator(const fw::func_type* _invokable)
{
    return create_abstract_function(_invokable, true);
}

Node* GraphNode::create_operator(const fw::iinvokable* _invokable)
{
	return create_function(_invokable, true);
}

void GraphNode::destroy(Node* _node)
{
    // disconnect any edge connected to this node
    std::vector<const DirectedEdge*> edges_to_disconnect;

    for (auto pair : m_edge_registry)
    {
        const DirectedEdge* edge = pair.second;
        if(edge->is_connected_to(_node) )
        {
            edges_to_disconnect.push_back(edge);
        }
    }
    for(auto each_edge: edges_to_disconnect)
    {
        disconnect(each_edge, false );
    };


    // if it is a variable, we remove it from its scope
    if ( VariableNode* node_variable = _node->as<VariableNode>() )
    {
        IScope* scope = node_variable->get_scope();
        if ( scope )
        {
            scope->remove_variable(node_variable);
        }
    }
    else if ( auto* scope = _node->get<Scope>() )
    {
        if ( !scope->has_no_variable() )
        {
            scope->remove_all_variables();
        }
    }

    // unregister and delete
    remove(_node);
    if ( _node == m_root )
    {
        m_root = nullptr;
    }
    delete _node;
}

bool GraphNode::is_empty() const
{
    return !m_root;
}

const DirectedEdge* GraphNode::connect(Property * _source_property, Property * _target_property)
{
    FW_EXPECT(_source_property != _target_property, "Can't connect same Property!")
    FW_EXPECT( fw::type::is_implicitly_convertible(_source_property->get_type(), _target_property->get_type()),
                       "Can't connect non implicitly convertible Properties!");

    const DirectedEdge* edge = nullptr;
    /*
     * If _from has no owner _to can digest it, no need to create an edge in this case.
     */
    if (_source_property->get_owner() == nullptr)
    {
        _target_property->digest(_source_property);
        delete _source_property;
    }
    else if (
            !_source_property->get_type().is_ptr() &&
            _source_property->get_owner()->get_type().is_child_of<LiteralNode>() &&
            _target_property->get_owner()->get_type().is_not_child_of<VariableNode>())
    {
        Node* owner = _source_property->get_owner();
        _target_property->digest(_source_property);
        destroy(owner);
    }
    else
    {
        LOG_VERBOSE("GraphNode", "drop_on() ...\n")
        _target_property->set_input(_source_property);
        _source_property->get_outputs().push_back(_target_property);

        edge = connect({_source_property, _target_property}, true);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        auto src_token = _source_property->get_src_token();
        if (src_token)
        {
            if (!_target_property->get_src_token())
            {
                _target_property->set_src_token(std::make_shared<Token>(src_token->m_type, "", src_token->m_source_word_pos));
            }
            _target_property->get_src_token()->transfer_prefix_suffix(src_token );
        }
    }

    set_dirty();

    return edge;
}

void GraphNode::remove(DirectedEdge* edge)
{
    auto found = std::find_if( m_edge_registry.begin()
                             , m_edge_registry.end()
                             , [edge](const EdgeRegistry_t::value_type& each){ return *edge == *each.second;});

    if (found != m_edge_registry.end() )
    {
        m_edge_registry.erase(found);
    }
    else
    {
        LOG_WARNING("GraphNode", "Unable to unregister edge\n")
    }
}

const DirectedEdge* GraphNode::connect(Node* _src, InstructionNode* _dst)
{
    // set declaration_instr once
    if(auto variable = _src->as<VariableNode>())
    {
        if( !variable->get_declaration_instr() )
        {
            variable->set_declaration_instr(_dst);
        }
    }

    return connect(_src->get_this_property(), _dst->get_root_node_property() );
}

const DirectedEdge* GraphNode::connect(Property * _src, VariableNode* _dst)
{
    return connect(_src, _dst->get_value() );
}

const DirectedEdge* GraphNode::connect(DirectedEdge _edge, bool _side_effects)
{
    auto edge = new DirectedEdge(_edge);
    Node* src = edge->prop.src->get_owner();
    Node* dst = edge->prop.dst->get_owner();

    switch (edge->type )
    {
        case Edge_t::IS_CHILD_OF:
        {
            /*
             * Here we create IS_SUCCESSOR_OF connections.
             */
            if ( _side_effects )
            {
                FW_ASSERT( dst->has<Scope>() )

                if (dst->successors().accepts() )                               // directly
                {
                    connect({src, Edge_t::IS_SUCCESSOR_OF, dst}, false);
                }
                else if ( Node* tail = dst->children_slots().get_back_or_nullptr() ) // to the last children
                {
                    if ( tail->has<Scope>() )
                    {
                        std::vector<InstructionNode*> tails;
                        Scope* scope = tail->get<Scope>();
                        scope->get_last_instructions_rec(tails);

                        for (InstructionNode *each_instruction : tails)
                        {
                            connect({src, Edge_t::IS_SUCCESSOR_OF, each_instruction}, false);
                        }

                        FW_ASSERT(!tails.empty())
                    }
                    else if (tail->successors().accepts() )
                    {
                        connect({src, Edge_t::IS_SUCCESSOR_OF, tail}, false);
                    }
                }
            }

            // create "parent-child" links
            dst->children_slots().add(src);
            src->set_parent(dst);

            break;
        }

        case Edge_t::IS_INPUT_OF:
            dst->inputs().add(src);
            src->outputs().add(dst);
            src->add_edge(edge);
            dst->add_edge(edge);

            break;

        case Edge_t::IS_SUCCESSOR_OF:
            dst->successors().add(src);
            src->predecessors().add(dst);

            if (_side_effects)
            {
                if ( dst->has<Scope>() )
                {
                    connect({src, Edge_t::IS_CHILD_OF, dst}, false);
                }
                else if ( Node* dst_parent = dst->get_parent() )
                {
                    connect({src, Edge_t::IS_CHILD_OF, dst_parent}, false);
                }

                /**
                 * create child/parent link with dst_parent
                 */
                if ( Node* src_parent = src->get_parent()  )
                {
                    Node *each_successor = src->successors().get_front_or_nullptr();
                    while (each_successor && each_successor->get_parent() == nullptr)
                    {
                        connect({each_successor, Edge_t::IS_CHILD_OF, src_parent}, false);
                        each_successor = each_successor->successors().get_front_or_nullptr();
                    }
                }
            }
            break;

        default:
            FW_ASSERT(false); // This connection type is not yet implemented
    }


    m_edge_registry.emplace(edge->type, edge);
    set_dirty();
    return edge;
}

void GraphNode::disconnect(const DirectedEdge* _edge, bool _side_effects)
{
    // find the edge to disconnect
    auto [begin, end] = m_edge_registry.equal_range(_edge->type );
    auto found = std::find_if(begin, end, [&_edge](auto& pair)
    {
        return pair.second->prop == _edge->prop; // we do not compare type, since we did a equal_range
    });

    if(found == end) return;

    m_edge_registry.erase(found);

    Node* src = _edge->prop.src->get_owner();
    Node* dst = _edge->prop.dst->get_owner();

    // disconnect effectively
    switch (_edge->type )
    {
        case Edge_t::IS_CHILD_OF:
            dst->children_slots().remove(src);
            src->set_parent(nullptr);
            break;

        case Edge_t::IS_INPUT_OF:
            dst->inputs().remove(src);
            src->outputs().remove(dst);
            src->remove_edge(_edge);
            dst->remove_edge(_edge);
            break;

        case Edge_t::IS_SUCCESSOR_OF:
            dst->successors().remove(src);
            src->predecessors().remove(dst);

            if ( _side_effects )
            {
                if ( auto parent = src->get_parent() )
                {
                    Node* successor = src;
                    while (successor && successor->get_parent() == parent )
                    {
                        DirectedEdge edge(successor, Edge_t::IS_CHILD_OF, parent);
                        disconnect(&edge, false );
                        successor = successor->successors().get_front_or_nullptr();
                    }
                }
            }
            break;

        default:
            FW_ASSERT(false); // This connection type is not yet implemented
    }

   set_dirty();
}

Node *GraphNode::create_scope()
{
    Node* scopeNode = m_factory->new_scope();
    add(scopeNode);
    return scopeNode;
}

ConditionalStructNode *GraphNode::create_cond_struct()
{
    ConditionalStructNode* condStructNode = m_factory->new_cond_struct();
    add(condStructNode);
    return condStructNode;
}

ForLoopNode* GraphNode::create_for_loop()
{
    ForLoopNode* for_loop = m_factory->new_for_loop_node();
    add(for_loop);
    return for_loop;
}

Node *GraphNode::create_root()
{
    m_root = m_factory->new_program();
    add(m_root);
    return m_root;
}

Node* GraphNode::create_node()
{
    Node* node = m_factory->new_node();
    add(node);
    return node;
}

LiteralNode* GraphNode::create_literal(fw::type _type)
{
    LiteralNode* node = m_factory->new_literal(_type);
    add(node);
    return node;
}

std::vector<const DirectedEdge*> GraphNode::filter_edges(Property* _property, Way _way) const
{
    std::vector<const DirectedEdge*> result;

    auto is_property_linked_to = [_property, _way](const DirectedEdge* edge)
    {
        return
            ( (_way & Way_Out) && edge->prop.src == _property)
            ||
            ( (_way & Way_In) && edge->prop.dst == _property);
    };

    for( auto& [type, each_edge] : m_edge_registry)
    {
        if ( is_property_linked_to(each_edge) ) result.push_back(each_edge);
    }

    return result;
}

