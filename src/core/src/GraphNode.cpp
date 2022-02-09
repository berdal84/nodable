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
            LOG_VERBOSE("GraphNode", "remove and delete: %s \n", node->getLabel() )
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

            if (node->needsToBeDeleted())
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
            if (each_node->isDirty())
            {
                each_node->eval();
                each_node->update();
                each_node->setDirty(false);
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
    _node->setParentGraph(this);
    LOG_VERBOSE("GraphNode", "registerNode %s (%s)\n", _node->getLabel(), _node->get_class()->get_name())
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
        if( wire->getSource()->getOwner() == _node || wire->getTarget()->getOwner() == _node )
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
    if (_src_member->getOwner() == nullptr)
    {
        _dst_member->digest(_src_member);
        delete _src_member;
    }
    else if (
            _src_member->getType() != Reflect::Type_Pointer &&
            _src_member->getOwner()->get_class()->is<LiteralNode>() &&
            _dst_member->getOwner()->get_class()->is_not<VariableNode>())
    {
        Node* owner = _src_member->getOwner();
        _dst_member->digest(_src_member);
        deleteNode(owner);
    }
    else
    {
        LOG_VERBOSE("GraphNode", "connect() ...\n")
        _dst_member->setInput(_src_member, _connect_by);
        _src_member->getOutputs().push_back(_dst_member);

        auto targetNode = _dst_member->getOwner()->as<Node>();
        auto sourceNode = _src_member->getOwner()->as<Node>();

        NODABLE_ASSERT(targetNode != sourceNode)

        // Link wire to members
        wire = this->newWire();

        wire->setSource(_src_member);
        wire->setTarget(_dst_member);

        LOG_VERBOSE("GraphNode", "connect() adding wire to nodes ...\n")
        targetNode->addWire(wire);
        sourceNode->addWire(wire);
        LOG_VERBOSE("GraphNode", "connect() wires added to node ...\n")

        connect(sourceNode, targetNode, Relation_t::IS_INPUT_OF);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        auto fromToken = _src_member->getSourceToken();
        if (fromToken) {
            if (!_dst_member->getSourceToken()) {
                _dst_member->setSourceToken(new Token(fromToken->m_type, "", fromToken->m_charIndex));
            }

            auto toToken = _dst_member->getSourceToken();
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

    this->setDirty();

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
    connect(_source, _target->value(), ConnectBy_Copy );
}

void GraphNode::connect(Node *_source, Node *_target, Relation_t _relationType, bool _sideEffects)
{
    switch ( _relationType )
    {
        case Relation_t::IS_CHILD_OF:
        {
            /*
             * Here we create IS_NEXT_OF connections.
             */
            if ( _sideEffects )
            {
                // First case is easy, if no children on the target node, the next node of the target IS the source.
                if (_target->has<Scope>() )
                {
                    if ( _target->get_children().empty() )
                    {
                        connect(_source, _target, Relation_t::IS_NEXT_OF, false);
                    }
                    else if ( _target->get_class()->is<ConditionalStructNode>() )
                    {
                        connect(_source, _target, Relation_t::IS_NEXT_OF, false);
                    }
                    else if ( !_target->get_children().back()->has<Scope>() )
                    {
                        connect(_source, _target->get_children().back(), Relation_t::IS_NEXT_OF, false);
                    }
                    else
                    {
                        auto& children = _target->get_children();
                        Node* back = children.back();
                        if (auto scope = back->get<Scope>() )
                        {
                            std::vector<InstructionNode *> last_instructions;
                            scope->get_last_instructions(last_instructions);
                            for (InstructionNode *each_instruction : last_instructions)
                            {
                                connect(_source, each_instruction, Relation_t::IS_NEXT_OF, false);
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
            _target->add_child(_source);
            _source->set_parent(_target);

            break;
        }

        case Relation_t::IS_INPUT_OF:
            _target->addInput(_source);
            _source->addOutput(_target);
            break;

        case Relation_t::IS_NEXT_OF:
            _target->addNext(_source);
            _source->addPrev(_target);

            if (_sideEffects)
            {
                if ( auto parent = _target->get_parent() )
                {
                    auto next = _source;
                    while ( next )
                    {
                        connect(next, parent, Relation_t::IS_CHILD_OF, false);
                        next = next->getFirstNext();
                    }
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }

    this->m_relationRegistry.emplace(_relationType, std::pair(_source, _target));
    this->setDirty();
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
            _target->remove_Child(_source);
            _source->set_parent(nullptr);
            break;

        case Relation_t::IS_INPUT_OF:
            _target->removeInput(_source);
            _source->removeOutput(_target);
            break;

        case Relation_t::IS_NEXT_OF:
            _target->removeNext(_source);
            _source->removePrev(_target);

            if ( _sideEffects )
            {
                if ( auto parent = _source->get_parent() )
                {
                    auto next = _source;
                    while ( next && next->get_parent() == parent )
                    {
                        disconnect(next, parent, Relation_t::IS_CHILD_OF, false );
                        next = next->getFirstNext();
                    }
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }

    // remove relation
    m_relationRegistry.erase(relation);

    this->setDirty();
}

void GraphNode::deleteWire(Wire *_wire)
{
    _wire->getTarget()->setInput(nullptr);
    auto& outputs = _wire->getSource()->getOutputs();
    outputs.erase( std::find(outputs.begin(), outputs.end(), _wire->getTarget()));

    Node* targetNode = _wire->getTarget()->getOwner();
    Node* sourceNode = _wire->getSource()->getOwner();

    if( targetNode )
        targetNode->removeWire(_wire);
    if( sourceNode )
        sourceNode->removeWire(_wire);

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

            Node *targetNode = wire->getTarget()->getOwner();
            Node *sourceNode = wire->getSource()->getOwner();

            targetNode->removeWire(wire);
            sourceNode->removeWire(wire);

            disconnect(sourceNode, targetNode, Relation_t::IS_INPUT_OF);

            deleteWire(wire);
        }
        else
        {
            ++it;
        }
    }

    setDirty();

}
