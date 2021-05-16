#include "GraphNode.h"

#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include "Log.h"
#include "Wire.h"
#include "Parser.h"
#include "ComputeBinaryOperation.h"
#include "ComputeUnaryOperation.h"
#include "DataAccess.h"
#include "Node.h"
#include "VariableNode.h"
#include "GraphTraversal.h"
#include "InstructionNode.h"
#include "CodeBlockNode.h"
#include "ScopedCodeBlockNode.h"
#include "ConditionalStructNode.h"
#include "LiteralNode.h"
#include "ProgramNode.h"
#include "AbstractNodeFactory.h"
#include "NodeView.h" // TODO: remove this dependency

using namespace Nodable::core;
using namespace Nodable::app; // TODO: remove this dependency

ImVec2 GraphNode::s_mainScopeView_lastKnownPosition = ImVec2(-1, -1); // draft try to store node position

GraphNode::~GraphNode()
{
	clear();
}

void GraphNode::clear()
{
	LOG_VERBOSE( "GraphNode", "=================== clear() ==================\n");

    if ( !m_wireRegistry.empty() )
    {
        for (auto it = m_wireRegistry.rbegin(); it != m_wireRegistry.rend(); it++)
        {
            deleteWire(*it);
        }
    }
    m_wireRegistry.clear();

	if ( !m_nodeRegistry.empty() )
	{
        for (auto i = m_nodeRegistry.size(); i > 0; i--)
        {
            Node* node = m_nodeRegistry[i - 1];
            LOG_VERBOSE("GraphNode", "remove and delete: %s \n", node->getLabel() );
            deleteNode(node);
        }
	}
    m_nodeRegistry.clear();
	m_relationRegistry.clear();
    m_program = nullptr;

    LOG_VERBOSE("GraphNode", "===================================================\n");
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
    if( m_program )
    {
        GraphTraversal traversal;
        auto updateResult = traversal.traverse(m_program, TraversalFlag_FollowInputs | TraversalFlag_FollowChildren |
                                                          TraversalFlag_FollowNotDirty | TraversalFlag_AvoidCycles);
        bool changed = false;
        for (Node *eachNode : traversal.getStats()
                .m_traversed)
        {
            if (eachNode->isDirty())
            {
                eachNode->eval();
                eachNode->update();
                eachNode->setDirty(false);
                changed |= true;
            }
        }

        if ( updateResult == Result::Success )
        {
            if ( changed )
            {
                traversal.logStats();
                result = UpdateResult::Success;
            }
            else
            {
                result = UpdateResult::SuccessWithoutChanges;
            }
        }
        else
        {
            result = UpdateResult::Failed;
        }
    }
    else
    {
        result = UpdateResult::SuccessWithoutChanges;
    }

    setDirty(false);
    return result;
}

void GraphNode::registerNode(Node* _node)
{
	this->m_nodeRegistry.push_back(_node);
    _node->setParentGraph(this);
    LOG_VERBOSE("GraphNode", "registerNode %s (%s)\n", _node->getLabel(), _node->getClass()->getName());
}

void GraphNode::unregisterNode(Node* _node)
{
    auto found = std::find(m_nodeRegistry.begin(), m_nodeRegistry.end(), _node);
    m_nodeRegistry.erase(found);
}

VariableNode* GraphNode::findVariable(std::string _name)
{
	return m_program->findVariable(_name);
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

VariableNode* GraphNode::newVariable(Type _type, const std::string& _name, ScopedCodeBlockNode* _scope)
{
	auto node = m_factory->newVariable(_type, _name, _scope);
    registerNode(node);

	return node;
}

Node* GraphNode::newOperator(const Operator* _operator)
{
    Node* node = m_factory->newOperator( _operator );

    if ( node )
    {
        registerNode(node);
    }

    return node;
}

Node* GraphNode::newBinOp(const Operator* _operator)
{
	Node* node = m_factory->newBinOp( _operator );
    registerNode(node);
	return node;
}

Node* GraphNode::newUnaryOp(const Operator* _operator)
{
	Node* node = m_factory->newUnaryOp( _operator );
    registerNode(node);

	return node;
}

Node* GraphNode::newFunction(const Function* _function)
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
        m_program(nullptr)
{
	this->clear();
}

CodeBlockNode *GraphNode::newCodeBlock()
{
    CodeBlockNode* codeBlockNode = m_factory->newCodeBlock();
    registerNode(codeBlockNode);
    return codeBlockNode;
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
    return m_program;
}

Wire *GraphNode::connect(Member* _from, Member* _to)
{
    Wire* wire = nullptr;

    /*
     * If _from has no owner _to can digest it, no Wire neede in that case.
     */
    if (_from->getOwner() == nullptr)
    {
        _to->digest(_from);
        delete _from;
    }
    else if (_from->getOwner()->getClass() == mirror::GetClass<LiteralNode>() &&
             _to->getOwner()->getClass() != mirror::GetClass<VariableNode>())
    {
        Node* owner = _from->getOwner();
        _to->digest(_from);
        deleteNode(owner);
    }
    else
    {
        _to->setInputMember(_from);
        auto targetNode = _to->getOwner()->as<Node>();
        auto sourceNode = _from->getOwner()->as<Node>();

        // Link wire to members
        wire = this->newWire();

        wire->setSource(_from);
        wire->setTarget(_to);

        targetNode->addWire(wire);
        sourceNode->addWire(wire);

        connect(sourceNode, targetNode, RelationType::IS_INPUT_OF);

        // TODO: move this somewhere else
        // (transfer prefix/suffix)
        auto fromToken = _from->getSourceToken();
        if (fromToken) {
            if (!_to->getSourceToken()) {
                _to->setSourceToken(new Token(fromToken->m_type, "", fromToken->m_charIndex));
            }

            auto toToken = _to->getSourceToken();
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
        LOG_WARNING("GraphNode", "Unable to unregister wire\n");
    }
}

void GraphNode::connect(Member* _source, InstructionNode* _target)
{
    connect(_source, _target->getValue());
}


void GraphNode::connect(Node *_source, Node *_target, RelationType _relationType, bool _sideEffects)
{
    switch ( _relationType )
    {
        case RelationType::IS_CHILD_OF:
        {
            if ( _sideEffects )
            {
                // create "next" links
                auto &target_children = _target->getChildren();
                if (!target_children.empty())
                {
                    auto lastChild = target_children.back();
                    auto lastChildParent = lastChild->getParent();
                    if (lastChildParent)
                    {
                        if (lastChildParent->getClass() == mirror::GetClass<ConditionalStructNode>())
                        {
                            connect(_source, _target, RelationType::IS_NEXT_OF, false);
                        }
                        else if (auto condStructNode = lastChild->as<ConditionalStructNode>())
                        {
                            // last instructions -> _source
                            std::vector<InstructionNode *> last_instr;
                            condStructNode->getLastInstructions(last_instr);

                            for (auto &each_inst : last_instr)
                            {
                                connect(_source, each_inst, RelationType::IS_NEXT_OF, false);
                            }
                        }
                        else
                        {
                            connect(_source, lastChild, RelationType::IS_NEXT_OF, false);
                        }
                    }
                    else
                    {
                        connect(_source, _target, RelationType::IS_NEXT_OF, false);
                    }
                }
                else
                {
                    connect(_source, _target, RelationType::IS_NEXT_OF, false);
                }
            }

            // create "parent-child" links
            _target->addChild(_source);
            _source->setParent(_target);

            if ( auto target_view = _target->getComponent<NodeView>())
            {
                if ( auto source_view = _source->getComponent<NodeView>())
                {
                    target_view->addChild(source_view);
                }
            }

            break;
        }

        case RelationType::IS_INPUT_OF:

            _target->addInput(_source);
            _source->addOutput(_target);

            if ( auto target_view = _target->getComponent<NodeView>())
            {
                if ( auto source_view = _source->getComponent<NodeView>())
                {
                    target_view->addInput(source_view);
                    source_view->addOutput(target_view);
                }
            }
            break;

        case RelationType::IS_NEXT_OF:
            _target->addNext(_source);
            _source->addPrev(_target);

            if (_sideEffects)
            {
                if (auto parent = _target->getParent())
                {
                    connect(_source, parent, RelationType::IS_CHILD_OF, false);
                }
            }
            break;

        default:
            NODABLE_ASSERT(false); // This connection type is not yet implemented
    }

    this->m_relationRegistry.emplace(_relationType, std::pair(_source, _target));
    this->setDirty();
}

void GraphNode::disconnect(Node *_source, Node *_target, RelationType _relationType)
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
        case RelationType::IS_CHILD_OF:
            _target->removeChild(_source);
            _source->setParent(nullptr);
            break;

        case RelationType::IS_INPUT_OF:
            _target->removeInput(_source);
            _source->removeOutput(_target);
            break;

        case RelationType::IS_NEXT_OF:
            _target->removeNext(_source);
            _source->removePrev(_target);
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
    _wire->getTarget()->setInputMember(nullptr);

    auto targetNode = _wire->getTarget()->getOwner();
    auto sourceNode = _wire->getSource()->getOwner();

    if( targetNode )
        targetNode->as<Node>()->removeWire(_wire);
    if( sourceNode )
        sourceNode->as<Node>()->removeWire(_wire);

    if( targetNode && sourceNode )
        disconnect(sourceNode->as<Node>(), targetNode->as<Node>(), RelationType::IS_INPUT_OF);

    delete _wire;
}

ScopedCodeBlockNode *GraphNode::newScopedCodeBlock()
{
    ScopedCodeBlockNode* scopeNode = m_factory->newScopedCodeBlock();
    registerNode(scopeNode);
    return scopeNode;
}

ConditionalStructNode *GraphNode::newConditionalStructure()
{
    ConditionalStructNode* condStructNode = m_factory->newConditionalStructure();
    registerNode(condStructNode);
    return condStructNode;
}

ScopedCodeBlockNode *GraphNode::newProgram()
{
    clear();
    m_program = m_factory->newProgram();
    registerNode(m_program);
    return m_program;
}

Node* GraphNode::newNode()
{
    Node* node = m_factory->newNode();
    registerNode(node);
    return node;
}

LiteralNode* GraphNode::newLiteral(const Type &type)
{
    LiteralNode* node = m_factory->newLiteral(type);
    registerNode(node);
    return node;
}
