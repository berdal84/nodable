#include <nodable/GraphNode.h>

#include <algorithm>    // for std::find_if
#include <nodable/Log.h>
#include <nodable/Wire.h>
#include <nodable/Parser.h>
#include <nodable/DataAccess.h>
#include <nodable/Node.h>
#include <nodable/VariableNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/INodeFactory.h>
#include <nodable/Scope.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(InstructionNode)

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

            if (node->needs_to_be_deleted())
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


    return result;
}

void GraphNode::add(Node* _node)
{
	m_node_registry.push_back(_node);
    _node->set_parent_graph(this);
    LOG_VERBOSE("GraphNode", "registerNode %s (%s)\n", _node->get_label(), _node->get_class()->get_name())
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

VariableNode* GraphNode::create_variable(Reflect::Type _type, const std::string& _name, IScope* _scope)
{
	auto node = m_factory->newVariable(_type, _name, _scope);
    add(node);
	return node;
}

Node* GraphNode::create_operator(const InvokableOperator* _operator)
{
    Node* node = m_factory->newOperator( _operator );

    if ( node )
    {
        add(node);
    }

    return node;
}

Node* GraphNode::create_bin_op(const InvokableOperator* _operator)
{
	Node* node = m_factory->newBinOp( _operator );
    add(node);
	return node;
}

Node* GraphNode::create_unary_op(const InvokableOperator* _operator)
{
	Node* node = m_factory->newUnaryOp( _operator );
    add(node);

	return node;
}

Node* GraphNode::create_function(const IInvokable* _function)
{
	Node* node = m_factory->newFunction( _function );
    add(node);
	return node;
}


Wire* GraphNode::create_wire()
{
	return new Wire();
}

void GraphNode::destroy(Node* _node)
{
    // delete any relation with this node
    for (auto it = m_wire_registry.begin(); it != m_wire_registry.end();)
    {
        Wire* wire = *it;
        if(wire->getSource()->get_owner() == _node || wire->getTarget()->get_owner() == _node )
        {
            destroy(wire);
            it = m_wire_registry.erase(it);
        }
        else
            it++;
    }

    // delete any relation with this node
    for (auto it = m_relation_registry.begin(); it != m_relation_registry.end();)
    {
        auto pair = (*it).second;
        if( pair.second == _node || pair.first == _node)
            it = m_relation_registry.erase(it);
        else
            it++;
    }

    // unregister and delete
    remove(_node);
    delete _node;
}

bool GraphNode::is_empty()
{
    return !m_root;
}

Wire *GraphNode::connect(Member* _src_member, Member* _dst, ConnBy_ _connect_by)
{
    Wire* wire = nullptr;

    /*
     * If _from has no owner _to can digest it, no Wire neede in that case.
     */
    if (_src_member->get_owner() == nullptr)
    {
        _dst->digest(_src_member);
        delete _src_member;
    }
    else if (
            _src_member->get_type() != Reflect::Type_Pointer &&
            _src_member->get_owner()->get_class()->is<LiteralNode>() &&
            _dst->get_owner()->get_class()->is_not<VariableNode>())
    {
        Node* owner = _src_member->get_owner();
        _dst->digest(_src_member);
        destroy(owner);
    }
    else
    {
        LOG_VERBOSE("GraphNode", "connect() ...\n")
        _dst->set_input(_src_member, _connect_by);
        _src_member->get_outputs().push_back(_dst);

        auto targetNode = _dst->get_owner()->as<Node>();
        auto sourceNode = _src_member->get_owner()->as<Node>();

        NODABLE_ASSERT(targetNode != sourceNode)

        // Link wire to members
        wire = create_wire();

        wire->setSource(_src_member);
        wire->setTarget(_dst);

        LOG_VERBOSE("GraphNode", "connect() adding wire to nodes ...\n")
        targetNode->add_wire(wire);
        sourceNode->add_wire(wire);
        LOG_VERBOSE("GraphNode", "connect() wires added to node ...\n")

        connect(sourceNode, targetNode, Relation_t::IS_INPUT_OF);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        auto fromToken = _src_member->get_src_token();
        if (fromToken) {
            if (!_dst->get_src_token()) {
                _dst->set_src_token(new Token(fromToken->m_type, "", fromToken->m_charIndex));
            }

            auto toToken = _dst->get_src_token();
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
    connect(_src->get_this_member(), _dst->get_root_node_member() );
}

void GraphNode::connect(Member* _src, VariableNode* _dst)
{
    // We connect the source member to the variable's value member in value mode (vs reference mode)
    connect(_src, _dst->get_value(), ConnectBy_Copy );
}

void GraphNode::connect(Node *_src, Node *_dst, Relation_t _relationType, bool _side_effects)
{
    switch ( _relationType )
    {
        case Relation_t::IS_PREDECESSOR_OF:
            return connect(_dst, _src, Relation_t::IS_SUCCESSOR_OF);

        case Relation_t::IS_CHILD_OF:
        {
            /*
             * Here we create IS_SUCCESSOR_OF connections.
             */
            if ( _side_effects )
            {
                // First case is easy, if no children on the target node, the next node of the target IS the source.
                if (_dst->has<Scope>() )
                {
                    if (_dst->children_slots().empty() )
                    {
                        connect(_src, _dst, Relation_t::IS_SUCCESSOR_OF, false);
                    }
                    else if ( _dst->get_class()->is<ConditionalStructNode>() )
                    {
                        connect(_src, _dst, Relation_t::IS_SUCCESSOR_OF, false);
                    }
                    else if ( !_dst->children_slots().back()->has<Scope>() )
                    {
                        connect(_src, _dst->children_slots().back(), Relation_t::IS_SUCCESSOR_OF, false);
                    }
                    else
                    {
                        auto& children = _dst->children_slots();
                        Node* back = children.back();
                        if (auto scope = back->get<Scope>() )
                        {
                            std::vector<InstructionNode *> last_instructions;
                            scope->get_last_instructions(last_instructions);
                            for (InstructionNode *each_instruction : last_instructions)
                            {
                                connect(_src, each_instruction, Relation_t::IS_SUCCESSOR_OF, false);
                            }
                        }
                    }

                }
                else
                {
                  NODABLE_ASSERT(false); // case missing
                }

            }

            // create "parent-child" links
            _dst->children_slots().add(_src);
            _src->set_parent(_dst);

            break;
        }

        case Relation_t::IS_INPUT_OF:
            _dst->input_slots().add(_src);
            _src->output_slots().add(_dst);
            break;

        case Relation_t::IS_SUCCESSOR_OF:
            _dst->successor_slots().add(_src);
            _src->predecessor_slots().add(_dst);

            if (_side_effects)
            {
                if ( auto parent = _dst->get_parent() )
                {
                    Node* successor = _src;
                    while ( successor )
                    {
                        connect(successor, parent, Relation_t::IS_CHILD_OF, false);
                        successor = successor->successor_slots().get_front_or_nullptr();
                    }
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }

    m_relation_registry.emplace(_relationType, std::pair(_src, _dst));
    set_dirty();
}

void GraphNode::disconnect(Node *_src, Node *_dst, Relation_t _relationType, bool _side_effects)
{
    NODABLE_ASSERT(_src && _dst);

    // find relation
    Relation pair{_relationType, {_src, _dst}};
    auto relation = std::find(m_relation_registry.begin(), m_relation_registry.end(), pair);

    if(relation == m_relation_registry.end())
        return;

    // disconnect effectively
    switch ( _relationType )
    {
        case Relation_t::IS_CHILD_OF:
            _dst->children_slots().remove(_src);
            _src->set_parent(nullptr);
            break;

        case Relation_t::IS_INPUT_OF:
            _dst->input_slots().remove(_src);
            _src->output_slots().remove(_dst);
            break;

        case Relation_t::IS_SUCCESSOR_OF:
            _dst->successor_slots().remove(_src);
            _src->predecessor_slots().remove(_dst);

            if ( _side_effects )
            {
                if ( auto parent = _src->get_parent() )
                {
                    Node* successor = _src;
                    while (successor && successor->get_parent() == parent )
                    {
                        disconnect(successor, parent, Relation_t::IS_CHILD_OF, false );
                        successor = successor->successor_slots().get_front_or_nullptr();
                    }
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }

    // remove relation
    m_relation_registry.erase(relation);

    set_dirty();
}

void GraphNode::destroy(Wire *_wire)
{
    _wire->getTarget()->set_input(nullptr);
    auto& outputs = _wire->getSource()->get_outputs();
    outputs.erase( std::find(outputs.begin(), outputs.end(), _wire->getTarget()));

    Node* targetNode = _wire->getTarget()->get_owner();
    Node* sourceNode = _wire->getSource()->get_owner();

    if( targetNode )
        targetNode->remove_wire(_wire);
    if( sourceNode )
        sourceNode->remove_wire(_wire);

    if( targetNode && sourceNode )
        disconnect(sourceNode->as<Node>(), targetNode->as<Node>(), Relation_t::IS_INPUT_OF);

    delete _wire;
}

Node *GraphNode::create_scope()
{
    Node* scopeNode = m_factory->newScope();
    add(scopeNode);
    return scopeNode;
}

ConditionalStructNode *GraphNode::create_cond_struct()
{
    ConditionalStructNode* condStructNode = m_factory->newConditionalStructure();
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
    m_root = m_factory->newProgram();
    add(m_root);
    return m_root;
}

Node* GraphNode::create_node()
{
    Node* node = m_factory->newNode();
    add(node);
    return node;
}

LiteralNode* GraphNode::create_literal(const Reflect::Type &type)
{
    LiteralNode* node = m_factory->newLiteral(type);
    add(node);
    return node;
}

void GraphNode::disconnect(Member *_member, Way _way)
{
    auto should_be_deleted = [&](const Wire* wire) {
        if ( (_way & Way_Out) && wire->getSource() == _member ) return true;
        if ( (_way & Way_In) && wire->getTarget() == _member ) return true;
        return false;
    };

    for (auto it = m_wire_registry.begin(); it != m_wire_registry.end(); )
    {
        if ( should_be_deleted(*it) )
        {
            auto wire = *it;
            it = m_wire_registry.erase(it);

            Node *targetNode = wire->getTarget()->get_owner();
            Node *sourceNode = wire->getSource()->get_owner();

            targetNode->remove_wire(wire);
            sourceNode->remove_wire(wire);

            disconnect(sourceNode, targetNode, Relation_t::IS_INPUT_OF);

            destroy(wire);
        }
        else
        {
            ++it;
        }
    }

    set_dirty();

}
