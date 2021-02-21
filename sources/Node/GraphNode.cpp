#include "GraphNode.h"
#include "Log.h"
#include "Language/Common/Parser.h"
#include "Node.h"
#include "VariableNode.h"
#include "ComputeBinaryOperation.h"
#include "ComputeUnaryOperation.h"
#include "Wire.h"
#include "WireView.h"
#include "DataAccess.h"
#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include "NodeView.h"
#include "Application.h"
#include "NodeTraversal.h"
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include "InstructionNode.h"
#include "CodeBlockNode.h"
#include "Node/ScopedCodeBlockNode.h"

using namespace Nodable;

ImVec2 GraphNode::LastResultNodeViewPosition = ImVec2(-1, -1); // draft try to store node position

GraphNode::~GraphNode()
{
	clear();
	delete scope;
}

void GraphNode::clear()
{
	// Store the Result node position to restore it later
	// TODO: handle multiple results
	if ( scope->hasInstructions() )
	{
		auto view = scope->getFirstInstruction()->getComponent<NodeView>();
        GraphNode::LastResultNodeViewPosition = view->getPosition();
	}

	LOG_VERBOSE( "GraphNode", "=================== clear() ==================\n");

	auto nodeIndex = nodes.size();

	while ( nodeIndex > 0)
    {
        nodeIndex--;
	    auto node = nodes.at(nodeIndex);
        LOG_VERBOSE("GraphNode", "remove and delete: %s \n", node->getLabel() );
        deleteNode(node);
	}
    nodes.resize(0);
    scope->clear();
    LOG_VERBOSE("GraphNode", "===================================================\n");
}

UpdateResult GraphNode::update()
{
    /*
        1 - Delete flagged Nodes
    */
    {
        auto nodeIndex = nodes.size();

        while (nodeIndex > 0)
        {
            nodeIndex--;
            auto node = nodes.at(nodeIndex);

            if (node->needsToBeDeleted())
            {
                this->deleteNode(node);
            }

        }
    }

	/*
	    2 - Update all Nodes
    */
    size_t updatedNodesCount(0);
    auto result = Result::Success;
    {
        auto it = nodes.begin();

        while (it < nodes.end() && result != Result::Failure)
        {
            auto node = *it;

            if (node && node->isDirty())
            {
                updatedNodesCount++;
                result = NodeTraversal::Update(node);
            }

            ++it;
        }
    }

	if( result != Result::Failure &&
	    updatedNodesCount > 0 && NodeView::GetSelected() != nullptr)
    {
	    return UpdateResult::Success;
    }
	else
    {
	    return UpdateResult::SuccessWithoutChanges;
    }

}

void GraphNode::registerNode(Node* _node)
{
	this->nodes.push_back(_node);
    _node->setParentGraph(this);
}

void GraphNode::unregisterNode(Node* _node)
{
    // TODO: remove Node from this->scope
    {
        auto it = std::find(nodes.begin(), nodes.end(), _node);
        if (it != nodes.end())
        {
            nodes.erase(it);
        }
    }
}

VariableNode* GraphNode::findVariable(std::string _name)
{
	return scope->findVariable(_name);
}

InstructionNode* GraphNode::newInstruction(CodeBlockNode* _parentCodeBlock)
{
	auto instructionNode = new InstructionNode(ICON_FA_SIGN_OUT_ALT " Result", _parentCodeBlock);
    _parentCodeBlock->pushInstruction(instructionNode);
    instructionNode->addComponent(new NodeView);
    this->registerNode(instructionNode);
	return instructionNode;
}

InstructionNode* GraphNode::newInstruction()
{
    std::string eol = language->getSerializer()->serialize(TokenType::EndOfLine);

    // add to code block
    if ( !scope->hasInstructions() )
    {
        scope->innerBlocs.push_back( reinterpret_cast<AbstractCodeBlockNode*>(newCodeBlock()) );
    }
    else
    {
        // insert an eol
        InstructionNode* lastInstruction = scope->getLastInstruction();
        lastInstruction->endOfInstructionToken->suffix += eol;
    }

    auto block = scope->getLastCodeBlock()->as<CodeBlockNode>();
    auto newInstructionNode = newInstruction(block);

    // Initialize (since it is a manual creation)
    Token* token = new Token(TokenType::EndOfInstruction);
    token->suffix = eol;
    newInstructionNode->endOfInstructionToken = token;

    return newInstructionNode;
}

VariableNode* GraphNode::newVariable(std::string _name, ScopedCodeBlockNode* _scope)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->setName(_name.c_str());
    this->registerNode(node);
	_scope->variables.push_back(node);
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
	const auto args = signature.getArgs();
	const Semantic* semantic = language->getSemantic();
	auto left   = node->add("lvalue", Visibility::Default, semantic->tokenTypeToType(args[0].type), Way_In);
	auto right  = node->add("rvalue", Visibility::Default, semantic->tokenTypeToType(args[1].type), Way_In);
	auto result = node->add("result", Visibility::Default, semantic->tokenTypeToType(signature.getType()), Way_Out);

	// Create ComputeBinaryOperation component and link values.
	auto binOpComponent = new ComputeBinaryOperation(_operator, language);	
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
	const auto args = signature.getArgs();
    const Semantic* semantic = language->getSemantic();
	auto left = node->add("lvalue", Visibility::Default, semantic->tokenTypeToType(args[0].type), Way_In);
	auto result = node->add("result", Visibility::Default, semantic->tokenTypeToType(signature.getType()), Way_Out);

	// Create ComputeBinaryOperation binOpComponent and link values.
	auto unaryOperationComponent = new ComputeUnaryOperation(_operator, language);
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
	node->setLabel(ICON_FA_CODE " " + _function->signature.getIdentifier());
    const Semantic* semantic = language->getSemantic();
	node->add("result", Visibility::Default, semantic->tokenTypeToType(_function->signature.getType()), Way_Out);

	// Create ComputeBase binOpComponent and link values.
	auto functionComponent = new ComputeFunction(_function, language);
	functionComponent->setResult(node->get("result"));

	// Arguments
	auto args = _function->signature.getArgs();
	for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
		std::string memberName = args[argIndex].name;
		auto member = node->add(memberName.c_str(), Visibility::Default, semantic->tokenTypeToType(args[argIndex].type), Way_In); // create node input
		functionComponent->setArg(argIndex, member); // link input to binOpComponent
	}	
	
	node->addComponent(functionComponent);
	node->addComponent(new NodeView());

    this->registerNode(node);

	return node;
}


Wire* GraphNode::newWire()
{
	Wire* wire = new Wire();
	wire->addComponent(new WireView);	
	return wire;
}

void GraphNode::arrangeNodeViews()
{
    if ( !scope->innerBlocs.empty())
    {
        auto* block = dynamic_cast<CodeBlockNode*>(scope->innerBlocs.front());

        for (auto it = block->instructionNodes.begin(); it != block->instructionNodes.end(); it++)
        {
            InstructionNode* instructionNode = *it;
            NodeView *nodeView = instructionNode->getComponent<NodeView>();

            // Store the Result node position to restore it later
            bool resultNodeHadPosition = GraphNode::LastResultNodeViewPosition.x != -1 &&
                                         GraphNode::LastResultNodeViewPosition.y != -1;

            if (nodeView && this->hasComponent<View>())
            {
                auto view = this->getComponent<View>();

                if (resultNodeHadPosition)
                {                                 /* if result node had a position stored, we restore it */
                    nodeView->setPosition(GraphNode::LastResultNodeViewPosition);
                    nodeView->translate(ImVec2(float(200) * (float)std::distance(block->instructionNodes.begin(), it), 0));
                }

                auto rect = view->getVisibleRect();
                if (!NodeView::IsInsideRect(nodeView, rect))
                {
                    ImVec2 defaultPosition = rect.GetCenter();
                    defaultPosition.x += rect.GetWidth() * 1.0f / 6.0f;
                    nodeView->setPosition(defaultPosition);
                }
            }
        }

        // TODO: handle multiple CodeBlockNode, not only the last (works for now because we have a single block).
        auto lastCodeBlock = this->scope->getLastCodeBlock()->as<CodeBlockNode>();
        NODABLE_ASSERT(lastCodeBlock != nullptr); // read to do above
        NodeView::ArrangeRecursively(lastCodeBlock);
    }
}

GraphNode::GraphNode(const Language* _language)
{
	language = _language;
    scope = new ScopedCodeBlockNode(nullptr);
}

CodeBlockNode *GraphNode::newCodeBlock()
{
    auto codeBlockNode = new CodeBlockNode(nullptr);
    std::string label = ICON_FA_SQUARE " Block " + std::to_string(this->scope->innerBlocs.size());
    codeBlockNode->setLabel(label);
    codeBlockNode->addComponent(new NodeView);

    this->registerNode(codeBlockNode);

    return codeBlockNode;
}

void GraphNode::deleteNode(Node* _node)
{
    unregisterNode(_node);
    delete _node;
}

bool GraphNode::hasInstructionNodes()
{
    return scope->hasInstructions();
}
