#include "GraphNode.h"

#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <Application.h>

#include "Core/Log.h"
#include "Core/Wire.h"
#include "Language/Common/Parser.h"
#include "Node/Node.h"
#include "Node/VariableNode.h"
#include "Component/ComputeBinaryOperation.h"
#include "Component/ComputeUnaryOperation.h"
#include "Component/WireView.h"
#include "Component/DataAccess.h"
#include "Component/NodeView.h"
#include "Component/GraphNodeView.h"
#include "Node/GraphTraversal.h"
#include "Node/InstructionNode.h"
#include "Node/CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/ConditionalStructNode.h"
#include "ProgramNode.h"
#include "VirtualMachine.h"

using namespace Nodable;

ImVec2 GraphNode::s_mainScopeView_lastKnownPosition = ImVec2(-1, -1); // draft try to store node position

GraphNode::~GraphNode()
{
	clear();
}

void GraphNode::clear()
{

	// Store the Result node position to restore it later
	// TODO: handle multiple results
	if (m_program && m_program->hasInstructions() )
	{
        auto view = m_program->getComponent<NodeView>();
        GraphNode::s_mainScopeView_lastKnownPosition = view->getPosition();
    }

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

    if ( auto view = this->getComponent<GraphNodeView>())
    {
        view->clearConstraints();
    }

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

    if ( this->isDirty())
    {
        // update view constraints
        if (auto view = getComponent<GraphNodeView>() )
        {
            view->updateViewConstraints();
        }
    }

    // update nodes
    UpdateResult result = UpdateResult::Failed;
    if (this->m_program && Application::s_instance && Application::s_instance->getVirtualMachine().isStopped() )
    {
        GraphTraversal traversal;
        auto updateResult = traversal.traverse(m_program, TraversalFlag_FollowInputs | TraversalFlag_FollowChildren | TraversalFlag_FollowNotDirty);
        bool changed = false;
        for(Node* eachNode : traversal.getStats().m_traversed )
        {
            if ( eachNode->isDirty() )
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
    }

    this->setDirty(false);
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
    // create
	auto instructionNode = new InstructionNode(ICON_FA_CODE " Instr.");
    instructionNode->addComponent(new NodeView);
    instructionNode->setShortLabel(ICON_FA_CODE);

    // register
    this->registerNode(instructionNode);

	return instructionNode;
}

InstructionNode* GraphNode::appendInstruction()
{
    std::string eol = m_language->getSerializer()->serialize(TokenType_EndOfLine);

    // add to code block
    if ( m_program->getChildren().empty())
    {
        connect(newCodeBlock(), m_program, RelationType::IS_CHILD_OF);
    }
    else
    {
        // insert an eol
        InstructionNode* lastInstruction = m_program->getLastInstruction();
        lastInstruction->getEndOfInstrToken()->m_suffix.append(eol );
    }

    auto block = m_program->getLastCodeBlock()->as<CodeBlockNode>();
    auto newInstructionNode = newInstruction();
    this->connect(newInstructionNode, block,  RelationType::IS_CHILD_OF);

    // Initialize (since it is a manual creation)
    Token* token = new Token(TokenType_EndOfInstruction);
    token->m_suffix = eol;
    newInstructionNode->setEndOfInstrToken( token );

    return newInstructionNode;
}

VariableNode* GraphNode::newVariable(std::string _name, ScopedCodeBlockNode* _scope)
{
    // create
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->setName(_name.c_str());

	// register
    this->registerNode(node);

    if( _scope)
    {
        _scope->addVariable(node);
    }
    else
    {
        LOG_WARNING("GraphNode", "You create a variable without defining its scope.");
    }

	return node;
}

VariableNode* GraphNode::newNumber(double _value)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->set(_value);
    this->registerNode(node);
	return node;
}

VariableNode* GraphNode::newNumber(const char* _value)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->set(std::stod(_value));
    this->registerNode(node);
	return node;
}

VariableNode* GraphNode::newString(const char* _value)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->set(_value);
    this->registerNode(node);
	return node;
}

Node* GraphNode::newOperator(const Operator* _operator)
{
    switch ( _operator->getType() )
    {
        case Operator::Type::Binary:
            return newBinOp(_operator);
        case Operator::Type::Unary:
            return newUnaryOp(_operator);
        default:
            return nullptr;
    }
}

Node* GraphNode::newBinOp(const Operator* _operator)
{
	// Create a node with 2 inputs and 1 output
	auto node = new Node();
	auto signature = _operator->signature;
	node->setLabel(signature.getLabel());
    node->setShortLabel(signature.getLabel().substr(0, 4).c_str());

    const auto args = signature.getArgs();
	const Semantic* semantic = m_language->getSemantic();
	auto props = node->getProps();
	auto left   = props->add("lvalue", Visibility::Default, semantic->tokenTypeToType(args[0].type), Way_In);
	auto right  = props->add("rvalue", Visibility::Default, semantic->tokenTypeToType(args[1].type), Way_In);
	auto result = props->add("result", Visibility::Default, semantic->tokenTypeToType(signature.getType()), Way_Out);

	// Create ComputeBinaryOperation component and link values.
	auto binOpComponent = new ComputeBinaryOperation(_operator, m_language);
	binOpComponent->setResult(result);	
	binOpComponent->setLValue( left );	
	binOpComponent->setRValue(right);
	node->addComponent(binOpComponent);

	// Create a NodeView component
	node->addComponent(new NodeView());

	// Add to this container
    this->registerNode(node);
		
	return node;
}

Node* GraphNode::newUnaryOp(const Operator* _operator)
{
	// Create a node with 2 inputs and 1 output
	auto node = new Node();
	auto signature = _operator->signature;
	node->setLabel(signature.getLabel());
    node->setShortLabel(signature.getLabel().substr(0, 4).c_str());
	const auto args = signature.getArgs();
    const Semantic* semantic = m_language->getSemantic();
    auto props = node->getProps();
    auto left = props->add("lvalue", Visibility::Default, semantic->tokenTypeToType(args[0].type), Way_In);
	auto result = props->add("result", Visibility::Default, semantic->tokenTypeToType(signature.getType()), Way_Out);

	// Create ComputeBinaryOperation binOpComponent and link values.
	auto unaryOperationComponent = new ComputeUnaryOperation(_operator, m_language);
	unaryOperationComponent->setResult(result);
	unaryOperationComponent->setLValue(left);
	node->addComponent(unaryOperationComponent);

	// Create a NodeView Component
	node->addComponent(new NodeView());

	// Add to this container
    this->registerNode(node);

	return node;
}

Node* GraphNode::newFunction(const Function* _function)
{
	// Create a node with 2 inputs and 1 output
	auto node = new Node();
	node->setLabel(_function->signature.getIdentifier() + "()");
	node->setShortLabel("f(x)");
    const Semantic* semantic = m_language->getSemantic();
	auto props = node->getProps();
	props->add("result", Visibility::Default, semantic->tokenTypeToType(_function->signature.getType()), Way_Out);

	// Create ComputeBase binOpComponent and link values.
	auto functionComponent = new ComputeFunction(_function, m_language);
	functionComponent->setResult(props->get("result"));

	// Arguments
	auto args = _function->signature.getArgs();
	for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
		std::string memberName = args[argIndex].name;
		auto member = props->add(memberName.c_str(), Visibility::Default, semantic->tokenTypeToType(args[argIndex].type), Way_In); // create node input
		functionComponent->setArg(argIndex, member); // link input to binOpComponent
	}	
	
	node->addComponent(functionComponent);
	node->addComponent(new NodeView());

    this->registerNode(node);

	return node;
}


Wire* GraphNode::newWire()
{
	return new Wire();
}

void GraphNode::arrangeNodeViews()
{
    if ( m_program ) {
        if (auto scopeView = m_program->getComponent<NodeView>()) {
            bool hasKnownPosition = GraphNode::s_mainScopeView_lastKnownPosition.x != -1 &&
                                    GraphNode::s_mainScopeView_lastKnownPosition.y != -1;

            if ( this->hasComponent<View>()) {
                auto view = this->getComponent<View>();

                if (hasKnownPosition) {                                 /* if result node had a position stored, we restore it */
                    scopeView->setPosition(GraphNode::s_mainScopeView_lastKnownPosition);
                }

                auto rect = view->getVisibleRect();
                if (!NodeView::IsInsideRect(scopeView, rect)) {
                    ImVec2 defaultPosition = rect.GetCenter();
                    defaultPosition.x += rect.GetWidth() * 1.0f / 6.0f;
                    scopeView->setPosition(defaultPosition);
                }
            }
        }

        m_program->getComponent<NodeView>()->arrangeRecursively(false);
    }
}

GraphNode::GraphNode(const Language* _language)
    :
        m_language(_language),
        m_program(nullptr)
{
	this->clear();
}

CodeBlockNode *GraphNode::newCodeBlock()
{
    auto codeBlockNode = new CodeBlockNode();
    std::string label = ICON_FA_CODE " Block " + std::to_string(this->m_program->getChildren().size());
    codeBlockNode->setLabel(label);
    codeBlockNode->setShortLabel(ICON_FA_CODE "Bl");
    codeBlockNode->addComponent(new NodeView);

    this->registerNode(codeBlockNode);

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

bool GraphNode::hasInstructionNodes()
{
    return m_program && m_program->hasInstructions();
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


void GraphNode::connect(Node *_source, Node *_target, RelationType _relationType)
{
    switch ( _relationType )
    {
        case RelationType::IS_CHILD_OF:
            _target->addChild(_source);
            _source->setParent(_target);
            break;

        case RelationType::IS_INPUT_OF:
            _target->addInput(_source);
            _source->addOutput(_target);
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
    auto scopeNode = new ScopedCodeBlockNode();
    std::string label = ICON_FA_CODE_BRANCH " Scope";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_CODE_BRANCH " Scop.");
    scopeNode->addComponent(new NodeView());
    this->registerNode(scopeNode);
    return scopeNode;
}

ConditionalStructNode *GraphNode::newConditionalStructure()
{
    auto scopeNode = new ConditionalStructNode();
    std::string label = ICON_FA_QUESTION " Condition";
    scopeNode->setLabel(label);
    scopeNode->setShortLabel(ICON_FA_QUESTION" Cond.");
    scopeNode->addComponent(new NodeView());
    this->registerNode(scopeNode);
    return scopeNode;
}

ScopedCodeBlockNode *GraphNode::newProgram() {
    clear();
    m_program = new ProgramNode();
    m_program->setLabel(ICON_FA_FILE_CODE " Program");
    m_program->setShortLabel(ICON_FA_FILE_CODE " Prog.");
    m_program->addComponent(new NodeView());
    registerNode(m_program);
    return this->m_program;
}

Node* GraphNode::newNode() {
    Node* node = new Node();
    registerNode(node);
    return node;
}
