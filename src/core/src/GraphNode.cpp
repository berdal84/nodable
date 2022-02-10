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
#include <nodable/AbstractNodeFactory.h>
#include <nodable/Scope.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(InstructionNode)

GraphNode::~GraphNode()
{
	clear();
}

void GraphNode::clear()
{
	LOG_VERBOSE( "GraphNode", "Clearing graph ...\n")

    if ( !m_wireRegistry.empty() )
    {
        for (auto it = m_wireRegistry.rbegin(); it != m_wireRegistry.rend(); it++)
        {
            deleteWire(*it);
        }
    }
    else
    {
        LOG_VERBOSE("GraphNode", "No wires in registry.\n")
    }
    m_wireRegistry.clear();

	if ( !m_nodeRegistry.empty() )
	{
        for (auto i = m_nodeRegistry.size(); i > 0; i--)
        {
            Node* node = m_nodeRegistry[i - 1];
            LOG_VERBOSE("GraphNode", "remove and delete: %s \n", node->get_label() )
            deleteNode(node);
        }
	}
	else
    {
        LOG_VERBOSE("GraphNode", "No nodes in registry.\n")
    }
    m_nodeRegistry.clear();
	m_relationRegistry.clear();
    m_program_root = nullptr;

    LOG_VERBOSE("GraphNode", "Graph cleared.\n")
}

UpdateResult GraphNode::update()
{
    // Delete flagged Nodes
    {
        auto nodeIndex = m_nodeRegistry.size();

        while (nodeIndex > 0)
        {
            nodeIndex--;
            auto node = m_nodeRegistry.at(nodeIndex);

            if (node->needs_to_be_deleted())
            {
                this->deleteNode(node);
            }

        }
    }

    // update nodes
    UpdateResult result;
    if( m_program_root )
    {
        bool changed = false;
        for (Node* each_node : m_nodeRegistry)
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

void GraphNode::registerNode(Node* _node)
{
	this->m_nodeRegistry.push_back(_node);
    _node->set_parent_graph(this);
    LOG_VERBOSE("GraphNode", "registerNode %s (%s)\n", _node->get_label(), _node->get_class()->get_name())
}

void GraphNode::unregisterNode(Node* _node)
{
    auto found = std::find(m_nodeRegistry.begin(), m_nodeRegistry.end(), _node);
    m_nodeRegistry.erase(found);
}

InstructionNode* GraphNode::newInstruction()
{
	auto instructionNode = m_factory->newInstruction();
    registerNode(instructionNode);

	return instructionNode;
}

InstructionNode* GraphNode::newInstruction_UserCreated()
{
    auto instructionNode = m_factory->newInstruction();
    registerNode(instructionNode);

    return instructionNode;
}

VariableNode* GraphNode::newVariable(Reflect::Type _type, const std::string& _name, AbstractScope* _scope)
{
	auto node = m_factory->newVariable(_type, _name, _scope);
    registerNode(node);

	return node;
}

Node* GraphNode::newOperator(const InvokableOperator* _operator)
{
    Node* node = m_factory->newOperator( _operator );

    if ( node )
    {
        registerNode(node);
    }

    return node;
}

Node* GraphNode::newBinOp(const InvokableOperator* _operator)
{
	Node* node = m_factory->newBinOp( _operator );
    registerNode(node);
	return node;
}

Node* GraphNode::newUnaryOp(const InvokableOperator* _operator)
{
	Node* node = m_factory->newUnaryOp( _operator );
    registerNode(node);

	return node;
}

Node* GraphNode::newFunction(const Invokable* _function)
{
	Node* node = m_factory->newFunction( _function );
    registerNode(node);
	return node;
}


Wire* GraphNode::newWire()
{
	return new Wire();
}

GraphNode::GraphNode(const Language* _language, const AbstractNodeFactory* _factory)
    :
        m_language(_language),
        m_factory(_factory),
        m_program_root(nullptr)
{
}

void GraphNode::deleteNode(Node* _node)
{
    // delete any relation with this node
    for (auto it = m_wireRegistry.begin(); it != m_wireRegistry.end();)
    {
        Wire* wire = *it;
        if(wire->getSource()->get_owner() == _node || wire->getTarget()->get_owner() == _node )
        {
            deleteWire(wire);
            it = m_wireRegistry.erase(it);
        }
        else
            it++;
    }

    // delete any relation with this node
    for (auto it = m_relationRegistry.begin(); it != m_relationRegistry.end();)
    {
        auto pair = (*it).second;
        if( pair.second == _node || pair.first == _node)
            it = m_relationRegistry.erase(it);
        else
            it++;
    }

    // unregister and delete
    unregisterNode(_node);
    delete _node;
}

bool GraphNode::hasProgram()
{
    return m_program_root;
}

Wire *GraphNode::connect(Member* _src_member, Member* _dst_member, ConnBy_ _connect_by)
{
    Wire* wire = nullptr;

    /*
     * If _from has no owner _to can digest it, no Wire neede in that case.
     */
    if (_src_member->get_owner() == nullptr)
    {
        _dst_member->digest(_src_member);
        delete _src_member;
    }
    else if (
            _src_member->get_type() != Reflect::Type_Pointer &&
            _src_member->get_owner()->get_class()->is<LiteralNode>() &&
            _dst_member->get_owner()->get_class()->is_not<VariableNode>())
    {
        Node* owner = _src_member->get_owner();
        _dst_member->digest(_src_member);
        deleteNode(owner);
    }
    else
    {
        LOG_VERBOSE("GraphNode", "connect() ...\n")
        _dst_member->set_input(_src_member, _connect_by);
        _src_member->get_outputs().push_back(_dst_member);

        auto targetNode = _dst_member->get_owner()->as<Node>();
        auto sourceNode = _src_member->get_owner()->as<Node>();

        NODABLE_ASSERT(targetNode != sourceNode)

        // Link wire to members
        wire = this->newWire();

        wire->setSource(_src_member);
        wire->setTarget(_dst_member);

        LOG_VERBOSE("GraphNode", "connect() adding wire to nodes ...\n")
        targetNode->add_wire(wire);
        sourceNode->add_wire(wire);
        LOG_VERBOSE("GraphNode", "connect() wires added to node ...\n")

        connect(sourceNode, targetNode, Relation_t::IS_INPUT_OF);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        auto fromToken = _src_member->get_src_token();
        if (fromToken) {
            if (!_dst_member->get_src_token()) {
                _dst_member->set_src_token(new Token(fromToken->m_type, "", fromToken->m_charIndex));
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
        registerWire(wire);
    }

    this->set_dirty();

    return wire;
}

void GraphNode::disconnect(Wire *_wire)
{
    unregisterWire(_wire);
    deleteWire(_wire);
}

void GraphNode::registerWire(Wire* _wire)
{
    m_wireRegistry.push_back(_wire);
}

void GraphNode::unregisterWire(Wire* _wire)
{
    auto found = std::find(m_wireRegistry.begin(), m_wireRegistry.end(), _wire);
    if (found != m_wireRegistry.end() )
    {
        m_wireRegistry.erase(found);
    }
    else
    {
        LOG_WARNING("GraphNode", "Unable to unregister wire\n")
    }
}

void GraphNode::connect(Node* _source, InstructionNode* _target)
{
    connect(_source->get_this_member(), _target->get_root_node_member() );
}

void GraphNode::connect(Member* _source, VariableNode* _target)
{
    // We connect the source member to the variable's value member in value mode (vs reference mode)
    connect(_source, _target->get_value(), ConnectBy_Copy );
}

void GraphNode::connect(Node *_source, Node *_target, Relation_t _relationType, bool _sideEffects)
{
    switch ( _relationType )
    {
        case Relation_t::IS_CHILD_OF:
        {
            /*
             * Here we create IS_SUCCESSOR_OF connections.
             */
            if ( _sideEffects )
            {
                // First case is easy, if no children on the target node, the next node of the target IS the source.
                if (_target->has<Scope>() )
                {
                    if (_target->children_slots().empty() )
                    {
                        connect(_source, _target, Relation_t::IS_SUCCESSOR_OF, false);
                    }
                    else if ( _target->get_class()->is<ConditionalStructNode>() )
                    {
                        connect(_source, _target, Relation_t::IS_SUCCESSOR_OF, false);
                    }
                    else if ( !_target->children_slots().back()->has<Scope>() )
                    {
                        connect(_source, _target->children_slots().back(), Relation_t::IS_SUCCESSOR_OF, false);
                    }
                    else
                    {
                        auto& children = _target->children_slots();
                        Node* back = children.back();
                        if (auto scope = back->get<Scope>() )
                        {
                            std::vector<InstructionNode *> last_instructions;
                            scope->get_last_instructions(last_instructions);
                            for (InstructionNode *each_instruction : last_instructions)
                            {
                                connect(_source, each_instruction, Relation_t::IS_SUCCESSOR_OF, false);
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
            _target->children_slots().add(_source);
            _source->set_parent(_target);

            break;
        }

        case Relation_t::IS_INPUT_OF:
            _target->input_slots().add(_source);
            _source->output_slots().add(_target);
            break;

        case Relation_t::IS_SUCCESSOR_OF:
            _target->successor_slots().add(_source);
            _source->predecessor_slots().add(_target);

            if (_sideEffects)
            {
                if ( auto parent = _target->get_parent() )
                {
                    Node* successor = _source;
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

    this->m_relationRegistry.emplace(_relationType, std::pair(_source, _target));
    this->set_dirty();
}

void GraphNode::disconnect(Node *_source, Node *_target, Relation_t _relationType, bool _sideEffects)
{
    NODABLE_ASSERT(_source && _target);

    // find relation
    Relation pair{_relationType, {_source, _target}};
    auto relation = std::find(m_relationRegistry.begin(), m_relationRegistry.end(), pair);

    if(relation == m_relationRegistry.end())
        return;

    // disconnect effectively
    switch ( _relationType )
    {
        case Relation_t::IS_CHILD_OF:
            _target->children_slots().remove(_source);
            _source->set_parent(nullptr);
            break;

        case Relation_t::IS_INPUT_OF:
            _target->input_slots().remove(_source);
            _source->output_slots().remove(_target);
            break;

        case Relation_t::IS_SUCCESSOR_OF:
            _target->successor_slots().remove(_source);
            _source->predecessor_slots().remove(_target);

            if ( _sideEffects )
            {
                if ( auto parent = _source->get_parent() )
                {
                    Node* successor = _source;
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
    m_relationRegistry.erase(relation);

    this->set_dirty();
}

void GraphNode::deleteWire(Wire *_wire)
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

Node *GraphNode::newScope()
{
    Node* scopeNode = m_factory->newScope();
    registerNode(scopeNode);
    return scopeNode;
}

ConditionalStructNode *GraphNode::newConditionalStructure()
{
    ConditionalStructNode* condStructNode = m_factory->newConditionalStructure();
    registerNode(condStructNode);
    return condStructNode;
}

ForLoopNode* GraphNode::new_for_loop_node()
{
    ForLoopNode* for_loop = m_factory->new_for_loop_node();
    registerNode(for_loop);
    return for_loop;
}

Node *GraphNode::newProgram()
{
    clear();
    m_program_root = m_factory->newProgram();
    registerNode(m_program_root);
    return m_program_root;
}

Node* GraphNode::newNode()
{
    Node* node = m_factory->newNode();
    registerNode(node);
    return node;
}

LiteralNode* GraphNode::newLiteral(const Reflect::Type &type)
{
    LiteralNode* node = m_factory->newLiteral(type);
    registerNode(node);
    return node;
}

void GraphNode::disconnect(Member *_member, Way _way)
{
    auto should_be_deleted = [&](const Wire* wire) {
        if ( (_way & Way_Out) && wire->getSource() == _member ) return true;
        if ( (_way & Way_In) && wire->getTarget() == _member ) return true;
        return false;
    };

    for ( auto it = m_wireRegistry.begin(); it != m_wireRegistry.end(); )
    {
        if ( should_be_deleted(*it) )
        {
            auto wire = *it;
            it = m_wireRegistry.erase(it);

            Node *targetNode = wire->getTarget()->get_owner();
            Node *sourceNode = wire->getSource()->get_owner();

            targetNode->remove_wire(wire);
            sourceNode->remove_wire(wire);

            disconnect(sourceNode, targetNode, Relation_t::IS_INPUT_OF);

            deleteWire(wire);
        }
        else
        {
            ++it;
        }
    }

    set_dirty();

}
