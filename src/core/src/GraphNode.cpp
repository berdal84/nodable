#include <nodable/core/GraphNode.h>

#include <algorithm>    // std::find_if

#include <nodable/core/Log.h>
#include <nodable/core/Wire.h>
#include <nodable/core/Parser.h>
#include <nodable/core/DataAccess.h>
#include <nodable/core/Node.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/INodeFactory.h>
#include <nodable/core/Scope.h>
#include <nodable/core/assertions.h>

using namespace Nodable;

REGISTER
{
    registration::push_class<GraphNode>("GraphNode").extends<Node>();
}

GraphNode::GraphNode(const Language* _language, const INodeFactory* _factory, const bool* _autocompletion)
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

    if ( !m_wire_registry.empty() )
    {
        for (auto it = m_wire_registry.rbegin(); it != m_wire_registry.rend(); it++)
        {
            destroy(*it);
        }
    }
    else
    {
        LOG_VERBOSE("GraphNode", "No wires in registry.\n")
    }
    m_wire_registry.clear();

	if ( !m_node_registry.empty() )
	{
        for (auto i = m_node_registry.size(); i > 0; i--)
        {
            Node* node = m_node_registry[i - 1];
            LOG_VERBOSE("GraphNode", "remove and delete: %s \n", node->get_label() )
            destroy(node);
        }
	}
	else
    {
        LOG_VERBOSE("GraphNode", "No nodes in registry.\n")
    }
    m_node_registry.clear();
	m_relation_registry.clear();
    m_root = nullptr;

    LOG_VERBOSE("GraphNode", "Graph cleared.\n")
}

UpdateResult GraphNode::update()
{
    // Delete flagged Nodes
    {
        auto nodeIndex = m_node_registry.size();

        while (nodeIndex > 0)
        {
            nodeIndex--;
            auto node = m_node_registry.at(nodeIndex);

            if (node->flagged_to_delete())
            {
                destroy(node);
            }

        }
    }

    // update nodes
    UpdateResult result;
    if( m_root )
    {
        bool changed = false;
        for (Node* each_node : m_node_registry)
        {
            if (each_node->is_dirty())
            {
                each_node->eval();
                each_node->update();
                each_node->set_dirty(false);
                changed |= true;
            }
        }
        if ( changed )
        {
            result = UpdateResult::Success;
        }
        else
        {
            result = UpdateResult::SuccessWithoutChanges;
        }

    }
    else
    {
        result = UpdateResult::SuccessWithoutChanges;
    }

    set_dirty(false);

    return result;
}

void GraphNode::add(Node* _node)
{
	m_node_registry.push_back(_node);
    _node->set_parent_graph(this);
    LOG_VERBOSE("GraphNode", "registerNode %s (%s)\n", _node->get_label(), _node->get_type().get_name())
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

VariableNode* GraphNode::create_variable(type _type, const std::string& _name, IScope* _scope)
{
    NODABLE_ASSERT(_type != type::null)

    auto node = m_factory->new_variable(_type, _name, _scope);
    add(node);
	return node;
}

Node* GraphNode::create_abstract_function(const Signature* _signature)
{
    if ( !_signature )
    {
        throw new std::runtime_error("unable to create_abstract_function, _signature is nullptr.");
    }
    Node* node = m_factory->new_abstract_function(_signature);
    add(node);
    return node;
}

Node* GraphNode::create_function(const IInvokable* _function)
{
    if ( !_function )
    {
        throw new std::runtime_error("unable to create_function, _function is nullptr.");
    }
	Node* node = m_factory->new_function(_function);
    add(node);
	return node;
}

void GraphNode::destroy(Node* _node)
{
    // delete any relation with this node
    for (auto it = m_wire_registry.begin(); it != m_wire_registry.end();)
    {
        Wire* wire = *it;
        if(wire->nodes.src == _node || wire->nodes.dst == _node )
        {
            destroy(wire);
            it = m_wire_registry.erase(it);
        }
        else
            it++;
    }

    // delete any relation with this node
    std::vector<DirectedEdge> relations_to_disconnect;

    for (auto pair :  m_relation_registry)
    {
        DirectedEdge relation = pair.second;
        if( relation.is_about(_node) )
        {
            relations_to_disconnect.push_back(relation);
        }
    }
    for(auto relation : relations_to_disconnect)
    {
        disconnect(relation, false );
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

Wire *GraphNode::connect(Member* _src_member, Member* _dst_member)
{
    NODABLE_ASSERT_EX(_src_member != _dst_member, "Can't connect same Member!")
    NODABLE_ASSERT_EX( type::is_implicitly_convertible(_src_member->get_type(), _dst_member->get_type()),
                       "Can't connect non implicitly convertible Members!");

    Wire* wire         = nullptr;
    /*
     * If _from has no owner _to can digest it, no Wire neede in that case.
     */
    if (_src_member->get_owner() == nullptr)
    {
        _dst_member->digest(_src_member);
        delete _src_member;
    }
    else if (
            !_src_member->get_type().is_ptr() &&
            _src_member->get_owner()->get_type().is_child_of<LiteralNode>() &&
            _dst_member->get_owner()->get_type().is_not_child_of<VariableNode>())
    {
        Node* owner = _src_member->get_owner();
        _dst_member->digest(_src_member);
        destroy(owner);
    }
    else
    {
        LOG_VERBOSE("GraphNode", "drop_on() ...\n")
        _dst_member->set_input(_src_member);
        _src_member->get_outputs().push_back(_dst_member);

        Node* dst_node = _dst_member->get_owner();
        Node* src_node = _src_member->get_owner();

        NODABLE_ASSERT_EX(dst_node != src_node, "Can't connect two members having same owner!")

        // Link wire to members
        wire = new Wire(_src_member, _dst_member);

        LOG_VERBOSE("GraphNode", "drop_on() adding wire to nodes ...\n")
        dst_node->add_wire(wire);
        src_node->add_wire(wire);
        LOG_VERBOSE("GraphNode", "drop_on() wires added to node ...\n")

        DirectedEdge relation(EdgeType::IS_INPUT_OF, src_node, dst_node);
        connect(relation);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        auto fromToken = _src_member->get_src_token();
        if (fromToken)
        {
            if (!_dst_member->get_src_token())
            {
                _dst_member->set_src_token(std::make_shared<Token>(fromToken->m_type, "", fromToken->m_charIndex));
            }

            auto toToken = _dst_member->get_src_token();
            toToken->m_suffix = fromToken->m_suffix;
            toToken->m_prefix = fromToken->m_prefix;
            fromToken->m_suffix = "";
            fromToken->m_prefix = "";
        }
    }

    if ( wire != nullptr )
    {
        add(wire);
    }

    set_dirty();

    return wire;
}

void GraphNode::disconnect(Wire *_wire)
{
    remove(_wire);
    destroy(_wire);
    set_dirty();
}

void GraphNode::add(Wire* _wire)
{
    m_wire_registry.push_back(_wire);
}

void GraphNode::remove(Wire* _wire)
{
    auto found = std::find(m_wire_registry.begin(), m_wire_registry.end(), _wire);
    if (found != m_wire_registry.end() )
    {
        m_wire_registry.erase(found);
    }
    else
    {
        LOG_WARNING("GraphNode", "Unable to unregister wire\n")
    }
}

void GraphNode::connect(Node* _src, InstructionNode* _dst)
{
    // set declaration_instr once
    if(auto variable = _src->as<VariableNode>())
    {
        if( !variable->get_declaration_instr() )
        {
            variable->set_declaration_instr(_dst);
        }
    }

    connect(_src->get_this_member(), _dst->get_root_node_member() );
}

void GraphNode::connect(Member* _src, VariableNode* _dst)
{
    connect(_src, _dst->get_value() );
}

void GraphNode::connect(DirectedEdge _relation, bool _side_effects)
{
    Node* src = _relation.nodes.src;
    Node* dst = _relation.nodes.dst;

    switch ( _relation.type )
    {
        case EdgeType::IS_CHILD_OF:
        {
            /*
             * Here we create IS_SUCCESSOR_OF connections.
             */
            if ( _side_effects )
            {
                NODABLE_ASSERT( dst->has<Scope>() )

                if (dst->successors().accepts() )                               // directly
                {
                    DirectedEdge relation(EdgeType::IS_SUCCESSOR_OF, src, dst);
                    connect(relation, false);
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
                            DirectedEdge relation(EdgeType::IS_SUCCESSOR_OF, src, each_instruction);
                            connect(relation, false);
                        }

                        NODABLE_ASSERT(!tails.empty())
                    }
                    else if (tail->successors().accepts() )
                    {
                        DirectedEdge relation(EdgeType::IS_SUCCESSOR_OF, src, tail);
                        connect(relation, false);
                    }
                }
            }

            // create "parent-child" links
            dst->children_slots().add(src);
            src->set_parent(dst);

            break;
        }

        case EdgeType::IS_INPUT_OF:
            dst->inputs().add(src);
            src->outputs().add(dst);
            break;

        case EdgeType::IS_SUCCESSOR_OF:
            dst->successors().add(src);
            src->predecessors().add(dst);

            if (_side_effects)
            {
                if ( dst->has<Scope>() )
                {
                    DirectedEdge relation(EdgeType::IS_CHILD_OF, src, dst);
                    connect(relation, false);
                }
                else if ( Node* dst_parent = dst->get_parent() )
                {
                    DirectedEdge relation(EdgeType::IS_CHILD_OF, src, dst_parent);
                    connect(relation, false);
                }

                /**
                 * create child/parent link with dst_parent
                 */
                if ( Node* src_parent = src->get_parent()  )
                {
                    Node *each_successor = src->successors().get_front_or_nullptr();
                    while (each_successor && each_successor->get_parent() == nullptr)
                    {
                        DirectedEdge relation(EdgeType::IS_CHILD_OF, each_successor, src_parent);
                        connect(relation, false);
                        each_successor = each_successor->successors().get_front_or_nullptr();
                    }
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }


    m_relation_registry.emplace(_relation.type, _relation);
    set_dirty();
}

void GraphNode::disconnect(DirectedEdge _relation, bool _side_effects)
{
    // find relation
    auto [begin, end] = m_relation_registry.equal_range( _relation.type );
    auto found = std::find_if(begin, end, [&_relation](auto& pair)
    {
        return pair.second.nodes == _relation.nodes; // we do not compare type, since we did a equal_range
    });

    if(found == end) return;

    m_relation_registry.erase(found);

    Node* src = _relation.nodes.src;
    Node* dst = _relation.nodes.dst;

    // disconnect effectively
    switch ( _relation.type )
    {
        case EdgeType::IS_CHILD_OF:
            dst->children_slots().remove(src);
            src->set_parent(nullptr);
            break;

        case EdgeType::IS_INPUT_OF:
            dst->inputs().remove(src);
            src->outputs().remove(dst);
            break;

        case EdgeType::IS_SUCCESSOR_OF:
            dst->successors().remove(src);
            src->predecessors().remove(dst);

            if ( _side_effects )
            {
                if ( auto parent = src->get_parent() )
                {
                    Node* successor = src;
                    while (successor && successor->get_parent() == parent )
                    {
                        DirectedEdge relation(EdgeType::IS_CHILD_OF, successor, parent);
                        disconnect(relation, false );
                        successor = successor->successors().get_front_or_nullptr();
                    }
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }

   set_dirty();
}

void GraphNode::destroy(Wire *_wire)
{
    Member* src_member = _wire->members.src;
    Member* dst_member = _wire->members.dst;
    Node*   src_node   = _wire->nodes.src;
    Node*   dst_node   = _wire->nodes.dst;

    dst_member->set_input(nullptr);

    auto& outputs = src_member->get_outputs();
    auto found = std::find(outputs.begin(), outputs.end(), dst_member);
    NODABLE_ASSERT_EX( found != outputs.end(), "dst_member not found in src_member´s outputs!")
    outputs.erase( found );

    if( dst_node ) dst_node->remove_wire(_wire);
    if( src_node ) src_node->remove_wire(_wire);

    if(dst_node && src_node )
    {
        DirectedEdge relation(EdgeType::IS_INPUT_OF, src_node, dst_node);
        disconnect(relation);
    }

    delete _wire;
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

LiteralNode* GraphNode::create_literal(type _type)
{
    LiteralNode* node = m_factory->new_literal(_type);
    add(node);
    return node;
}

std::vector<Wire*> GraphNode::filter_wires(Member* _member, Way _way) const
{
    std::vector<Wire*> result;

    auto is_member_linked_to = [_member, _way](const Wire* wire)
    {
        return
            ( (_way & Way_Out) && wire->members.src == _member )
            ||
            ( (_way & Way_In) && wire->members.dst == _member );
    };

    for(Wire* each_wire : m_wire_registry)
    {
        if ( is_member_linked_to(each_wire) ) result.push_back(each_wire);
    }

    return result;
}

void GraphNode::disconnect(Member *_member, Way _way, bool _side_effects)
{
    auto wires_to_delete = filter_wires(_member, _way);

    for (Wire* wire : wires_to_delete )
    {
        m_wire_registry.erase( std::find(m_wire_registry.begin(), m_wire_registry.end(), wire));

        Node* dst_node = wire->nodes.dst;
        Node* src_node = wire->nodes.src;

        dst_node->remove_wire(wire);
        src_node->remove_wire(wire);

        if ( _side_effects)
        {
            DirectedEdge relation(EdgeType::IS_INPUT_OF, src_node, dst_node);
            disconnect(relation, false);
        }

        destroy(wire);
    }

    set_dirty();

}
