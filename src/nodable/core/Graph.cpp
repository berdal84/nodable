#include "Graph.h"

#include <algorithm>    // std::find_if

#include "ConditionalStructNode.h"
#include "DirectedEdge.h"
#include "NodeFactory.h"
#include "InstructionNode.h"
#include "LiteralNode.h"
#include "NodeUtils.h"
#include "Node.h"
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
    std::vector<const DirectedEdge*> edges_to_disconnect;

    for (auto& pair : m_edge_registry)
    {
        const DirectedEdge* edge = pair.second;
        if(edge->is_connected_to( _node->id() ) )
        {
            edges_to_disconnect.push_back(edge);
        }
    }
    for(auto each_edge: edges_to_disconnect)
    {
        disconnect(each_edge, false );
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

const DirectedEdge* Graph::connect(Property *_source_property, Property *_target_property)
{
    FW_EXPECT(_source_property != _target_property, "Can't connect same Property!")
    FW_EXPECT( fw::type::is_implicitly_convertible(_source_property->get_type(), _target_property->get_type()),
                       "Can't connect non implicitly convertible Properties!");

    const DirectedEdge* edge = nullptr;
    /*
     * If _from has no owner _to can digest it, no need to create an edge in this case.
     */
    if (_source_property->owner().get() == nullptr )
    {
        _target_property->digest(_source_property);
        delete _source_property;
    }
    else if (
            !_source_property->is_referencing_a_node() &&
            _source_property->owner()->get_type()->is_child_of<LiteralNode>() &&
            _target_property->owner()->get_type()->is_not_child_of<VariableNode>())
    {
        ID<Node> source_owner = _source_property->owner();
        _target_property->digest(_source_property);
        destroy(source_owner);
    }
    else
    {
        LOG_VERBOSE("Graph", "drop_on() ...\n")
        _target_property->set_input(_source_property);
        _source_property->get_outputs().push_back(_target_property);

        edge = connect({_source_property, _target_property}, true);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        Token* src_token = &_source_property->token;
        if (!src_token->is_null())
        {
            if (!_target_property->token.is_null())
            {
                _target_property->token.clear();
                _target_property->token.m_type = src_token->m_type;
            }
            _target_property->token.transfer_prefix_and_suffix_from(src_token);
        }
    }

    set_dirty();

    return edge;
}

void Graph::remove(DirectedEdge* edge)
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
        LOG_WARNING("Graph", "Unable to unregister edge\n")
    }
}

const DirectedEdge* Graph::connect(Node *_src, InstructionNode *_dst)
{
    // set declaration_instr once
    if ( auto* variable = fw::cast<VariableNode>(_src) )
    {
        if( variable->get_declaration_instr().get() == nullptr )
        {
            variable->set_declaration_instr(_dst->id());
        }
    }
    return connect(_src->as_prop(), _dst->root() );
}

const DirectedEdge* Graph::connect(Property* _src, VariableNode *_dst)
{
    return connect(_src, _dst->property() );
}

const DirectedEdge* Graph::connect(DirectedEdge _edge, bool _side_effects)
{
    auto edge = new DirectedEdge(_edge);
    auto [src, dst] = edge->nodes();

    switch ( edge->type() )
    {
        case Edge_t::IS_CHILD_OF:
        {
            /*
             * Here we create IS_SUCCESSOR_OF connections.
             */
            if ( _side_effects )
            {
                FW_ASSERT(dst->has_component<Scope>() )

                if (dst->successors.accepts() )                               // directly
                {
                    connect({src, Edge_t::IS_SUCCESSOR_OF, dst}, false);
                }
                else if ( Node* tail = dst->children.last().get() ) // to the last children
                {
                    if ( auto scope = tail->get_component<Scope>().get() )
                    {
                        std::vector<InstructionNode*> tails;
                        scope->get_last_instructions_rec(tails);

                        for (InstructionNode* each_instruction : tails)
                        {
                            connect({src, Edge_t::IS_SUCCESSOR_OF, each_instruction}, false);
                        }

                        if( !tails.empty()) LOG_VERBOSE("Graph", "Empty scope found when trying to connect(...)" );
                    }
                    else if (tail->successors.accepts() )
                    {
                        connect({src, Edge_t::IS_SUCCESSOR_OF, tail}, false);
                    }
                }
            }

            // create "parent-child" links
            dst->children.add( src->id() );
            src->set_parent( dst->id() );

            break;
        }

        case Edge_t::IS_INPUT_OF:
            dst->inputs.add( src->id() );
            src->outputs.add( dst->id() );
            src->add_edge( edge );
            dst->add_edge( edge );

            break;

        case Edge_t::IS_SUCCESSOR_OF:
            dst->successors.add( src->id() );
            src->predecessors.add( dst->id() );

            if (_side_effects)
            {
                if (dst->has_component<Scope>() )
                {
                    connect({src, Edge_t::IS_CHILD_OF, dst}, false);
                }
                else if ( Node* dst_parent = dst->parent.get() )
                {
                    connect({src, Edge_t::IS_CHILD_OF, dst_parent}, false);
                }

                /**
                 * create child/parent link with dst_parent
                 */
                if ( Node* src_parent = src->parent.get()  )
                {
                    Node* each_successor = src->successors.first().get();
                    while (each_successor && each_successor->parent.get() != nullptr )
                    {
                        connect({each_successor, Edge_t::IS_CHILD_OF, src_parent }, false);
                        each_successor = each_successor->successors.first().get();
                    }
                }
            }
            break;

        default:
            FW_ASSERT(false); // This connection type is not yet implemented
    }


    m_edge_registry.emplace(edge->type(), edge);
    set_dirty();
    return edge;
}

void Graph::disconnect(const DirectedEdge* _edge, bool _side_effects)
{
    // find the edge to disconnect
    auto [begin, end] = m_edge_registry.equal_range(_edge->type() );
    auto found = std::find_if(begin, end, [&_edge](auto& pair)
    {
        return pair.second->props() == _edge->props(); // we do not compare type, since we did a equal_range
    });

    if(found == end) return;

    m_edge_registry.erase(found);

    auto [src, dst] = _edge->nodes();

    // disconnect effectively
    switch (_edge->type() )
    {
        case Edge_t::IS_CHILD_OF:
            dst->children.remove( src->id() );
            src->set_parent({});
            break;

        case Edge_t::IS_INPUT_OF:
            dst->inputs.remove( src->id() );
            src->outputs.remove( dst->id() );
            src->remove_edge(_edge);
            dst->remove_edge(_edge);
            break;

        case Edge_t::IS_SUCCESSOR_OF:
        {
            Node* successor = src;
            dst->successors.remove( successor->id() );
            successor->predecessors.remove( dst->id() );
            Node* successor_parent = successor->parent.get();
            if ( _side_effects && successor_parent  )
            {
                while (successor && successor->parent == src->parent )
                {
                    DirectedEdge edge(successor, Edge_t::IS_CHILD_OF, src->parent.get());
                    disconnect(&edge, false );
                    successor = successor->successors.first().get();
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

std::vector<const DirectedEdge*> Graph::filter_edges(Property* property, Way _way) const
{
    std::vector<const DirectedEdge*> result;
    auto is_property_linked_to = [property, _way](const DirectedEdge* edge)
    {
        return
            ( (_way & Way_Out) && edge->src() == property)
            ||
            ( (_way & Way_In) && edge->dst() == property);
    };

    for( auto& [type, each_edge] : m_edge_registry)
    {
        if ( is_property_linked_to(each_edge) ) result.push_back(each_edge);
    }

    return result;
}

