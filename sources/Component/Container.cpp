#include "Container.h"
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

using namespace Nodable;

ImVec2 Container::LastResultNodeViewPosition = ImVec2(-1, -1); // draft try to store node position

Container::~Container()
{
	clear();
	delete scope;
}

void Container::clear()
{
	// Store the Result node position to restore it later
	// TODO: handle multiple results
	if ( scope->hasInstructions() )
	{
		auto view = scope->getFirstInstruction()->getComponent<NodeView>();
		Container::LastResultNodeViewPosition = view->getRoundedPosition();
	}

	LOG_VERBOSE( "Container", "=================== clear() ==================\n");

	auto nodeIndex = nodes.size();

	while ( nodeIndex > 0)
    {
        nodeIndex--;
	    auto node = nodes.at(nodeIndex);
        LOG_VERBOSE("Container", "remove and delete: %s \n", node->getLabel() );
        remove(node);
        delete node;
	}
    nodes.resize(0);
    scope->clear();
    LOG_VERBOSE("Container", "===================================================\n");
}

UpdateResult Container::update()
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
                remove(node);
                delete node;
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

void Container::add(Node* _node)
{
	this->nodes.push_back(_node);
	_node->setParentContainer(this);
}

void Container::remove(Node* _node)
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

VariableNode* Container::findVariable(std::string _name)
{
	return scope->findVariable(_name);
}

InstructionNode* Container::newInstruction()
{
	auto instructionNode = new InstructionNode(ICON_FA_SIGN_OUT_ALT " Result");
    instructionNode->addComponent(new NodeView);
	this->add(instructionNode);
	return instructionNode;
}

VariableNode* Container::newVariable(std::string _name, ScopedCodeBlock* _scope)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->setName(_name.c_str());
	this->add(node);
	_scope->variables.push_back(node);
	return node;
}

VariableNode* Container::newNumber(double _value)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->set(_value);
	this->add(node);
	return node;
}

VariableNode* Container::newNumber(const char* _value)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->set(std::stod(_value));
	this->add(node);
	return node;
}

VariableNode* Container::newString(const char* _value)
{
	auto node = new VariableNode();
	node->addComponent( new NodeView);
	node->set(_value);
	this->add(node);
	return node;
}

Node* Container::newOperator(const Operator* _operator)
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

Node* Container::newBinOp(const Operator* _operator)
{
	// CREATE THE NODE :
	//------------------

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
	this->add(node);
		
	return node;
}

Node* Container::newUnaryOp(const Operator* _operator)
{
	// CREATE THE NODE :
	//------------------

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
	this->add(node);

	return node;
}

Node* Container::newFunction(const Function* _function) {

	// CREATE THE NODE :
	//------------------

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

	this->add(node);

	return node;
}


Wire* Container::newWire()
{
	Wire* wire = new Wire();
	wire->addComponent(new WireView);	
	return wire;
}

void Container::arrangeResultNodeViews()
{
    // TODO: fix this function, traverse the scope recursively.

//    std::vector<InstructionNode*> results;
//
//    for (auto it = results.begin(); it != results.end(); it++)
//    {
    auto instruction = scope->getFirstInstruction();
    if ( !instruction)
    {
        return;
    }

        NodeView *nodeView = instruction->getComponent<NodeView>();

        // Store the Result node position to restore it later
        bool resultNodeHadPosition = Container::LastResultNodeViewPosition.x != -1 &&
                                     Container::LastResultNodeViewPosition.y != -1;

        if (nodeView && this->hasComponent<View>())
        {
            auto view = this->getComponent<View>();

            if (resultNodeHadPosition)
            {                                 /* if result node had a position stored, we restore it */
                nodeView->setPosition(Container::LastResultNodeViewPosition);
//                nodeView->translate(ImVec2(float(200) * (float)std::distance(results.begin(), it), 0));
            }

            auto rect = view->getVisibleRect();
            if (!NodeView::IsInsideRect(nodeView, rect))
            {
                ImVec2 defaultPosition = rect.GetCenter();
                defaultPosition.x += rect.GetWidth() * 1.0f / 6.0f;
                nodeView->setPosition(defaultPosition);
            }
        }
//    }
}

size_t Container::getNodeCount()const
{
	return nodes.size();
}

Container::Container(const Language* _language)
{
	language = _language;
    scope = new ScopedCodeBlock(nullptr);
}

const Language *Container::getLanguage()const {
    return language;
}

bool Container::hasInstructions()
{
    return false;
}
