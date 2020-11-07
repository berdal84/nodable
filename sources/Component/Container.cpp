#include "Container.h"
#include "Log.h"
#include "Parser.h"
#include "Node.h"
#include "Variable.h"
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

using namespace Nodable;

ImVec2 Container::LastResultNodePosition = ImVec2(-1, -1); // draft try to store node position

Container::~Container()
{
	clear();
}

void Container::clear()
{
    LOG_MESSAGE(1u, "Container::clear() - // start\n");

	// Store the Result node position to restore it later
	if ( resultNode.get() )
	{
		auto view = resultNode->getComponent<NodeView>();
		Container::LastResultNodePosition = view->getRoundedPosition();
	}

	variables.clear();
    nodes.clear();
    resultNode.reset();

    LOG_MESSAGE(1u, "Container::clear() - // end\n");
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
            auto node = it->get();

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
	    return UpdateResult::SuccessWithChanges;
    }
	else
    {
	    return UpdateResult::SuccessWithoutChanges;
    }

}

void Container::add(std::shared_ptr<Node> _node)
{
	this->nodes.push_back( _node );
	_node->setParentContainer(this);
}

void Container::remove(const std::shared_ptr<Node> _node)
{

    if ( auto nodeAsVariable = std::static_pointer_cast<Variable>(_node) )
    {
        auto it = variables.find(nodeAsVariable->getName());
        if (it != variables.end())
        {
            variables.erase(it);
        }
    }

    {
        auto it = std::find(nodes.begin(), nodes.end(), _node);
        if (it != nodes.end())
        {
            nodes.erase(it);
        }
    }

    if (_node == this->resultNode)
    {
        this->resultNode.reset();
    }
}

Variable* Container::findVariable(std::string _name)
{
	auto pair = variables.find(_name);

	if ( pair != variables.end() )
    {
	    return pair->second;
    }

	return nullptr;
}

Variable* Container::newResult()
{
	auto variable = newVariable(ICON_FA_SIGN_OUT_ALT " Result");
	auto member = variable->get("value");
	member->setConnectorWay(Way_In);                     // disable output because THIS node is the output !
	resultNode = variable;
	return variable.get();
}

std::shared_ptr<Variable> Container::newVariable(std::string _name)
{
	auto node = std::make_shared<Variable>();
	node->newComponent<NodeView>();
	node->setName(_name.c_str());
	this->add(node);

    auto alreadyExisting = this->findVariable(_name);
    if ( alreadyExisting )
    {
        throw std::runtime_error( "Unable to create a variable because a variable with the same name already exists\n");
    }
    this->variables.insert_or_assign(_name, node.get());

	return node;
}

Variable* Container::newNumber(double _value)
{
	auto node = std::make_shared<Variable>();
	node->newComponent<NodeView>();
	node->set(_value);
	this->add(node);
	return node.get();
}

Variable* Container::newNumber(const char* _value)
{
	auto node = std::make_shared<Variable>();
    node->newComponent<NodeView>();
	node->set(std::stod(_value));
	this->add(node);
	return node.get();
}

Variable* Container::newString(const char* _value)
{
	auto node = std::make_shared<Variable>();
    node->newComponent<NodeView>();
	node->set(_value);
	this->add(node);
	return node.get();
}


Node* Container::newBinOp(const Operator* _operator)
{
	// CREATE THE NODE :
	//------------------

	// Create a node with 2 inputs and 1 output
	auto node = std::make_shared<Node>();
	auto signature = _operator->signature;
	node->setLabel(signature.getLabel());
	const auto args = signature.getArgs();
	auto left   = node->add("lvalue", Visibility::Default, language->tokenTypeToType(args[0].type), Way_In);
	auto right  = node->add("rvalue", Visibility::Default, language->tokenTypeToType(args[1].type), Way_In);
	auto result = node->add("result", Visibility::Default, language->tokenTypeToType(signature.getType()), Way_Out);

	// Create ComputeBinaryOperation component and link values.
	auto binOpComponent = node->newComponent<ComputeBinaryOperation>().lock();
	binOpComponent->setLanguage(language);
	binOpComponent->setFunction(_operator);
	binOpComponent->setResult(result);	
	binOpComponent->setLValue(left);
	binOpComponent->setRValue(right);

	// Create a NodeView component
    node->newComponent<NodeView>();

	// Add to this container
	this->add(node);
		
	return node.get();
}

Node* Container::newUnaryOp(const Operator* _operator)
{
	// CREATE THE NODE :
	//------------------

	// Create a node with 2 inputs and 1 output
	auto node = std::make_shared<Node>();
	auto signature = _operator->signature;
	node->setLabel(signature.getLabel());
	const auto args = signature.getArgs();
	auto left = node->add("lvalue", Visibility::Default, language->tokenTypeToType(args[0].type), Way_In);
	auto result = node->add("result", Visibility::Default, language->tokenTypeToType(signature.getType()), Way_Out);

	// Create ComputeBinaryOperation binOpComponent and link values.
	auto unaryOperationComponent = node->newComponent<ComputeUnaryOperation>().lock();
	unaryOperationComponent->setLanguage(language);
	unaryOperationComponent->setFunction(_operator);
	unaryOperationComponent->setResult(result);
	unaryOperationComponent->setLValue(left);

	// Create a NodeView Component
    node->newComponent<NodeView>();

	// Add to this container
	this->add(node);

	return node.get();
}

Node* Container::newFunction(const Function* _function) {

	// CREATE THE NODE :
	//------------------

	// Create a node with 2 inputs and 1 output
	auto node = std::make_shared<Node>();
	node->setLabel(ICON_FA_CODE " " + _function->signature.getIdentifier());
	node->add("result", Visibility::Default, language->tokenTypeToType(_function->signature.getType()), Way_Out);

	// Create ComputeBase binOpComponent and link values.
	auto computeFunctionComponent = node->newComponent<ComputeFunction>().lock();
    computeFunctionComponent->setLanguage(this->language);
	computeFunctionComponent->setFunction(_function);
	computeFunctionComponent->setResult(node->get("result"));

	auto args = _function->signature.getArgs();
	for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
		std::string memberName = args[argIndex].name;
		auto member = node->add(memberName.c_str(), Visibility::Default, language->tokenTypeToType(args[argIndex].type), Way_In); // create node input
		computeFunctionComponent->setArg(argIndex, member); // link input to binOpComponent
	}	
	
	// Add a node view
    node->newComponent<NodeView>();

	this->add(node);

	return node.get();
}


std::shared_ptr<Wire> Container::newWire()
{
	auto wire = std::make_shared<Wire>();
	wire->newComponent<WireView>();
	return wire;
}

void Container::tryToRestoreResultNodePosition()
{
    if ( resultNode.get() )
    {
        // Store the Result node position to restore it later
        auto nodeView = resultNode->getComponent<NodeView>();
        bool resultNodeHadPosition = Container::LastResultNodePosition.x != -1 &&
                                     Container::LastResultNodePosition.y != -1;

        if (nodeView && this->hasComponent<View>()) {

            auto view = this->getComponent<View>();

            if (resultNodeHadPosition) {                                 /* if result node had a position stored, we restore it */
                nodeView->setPosition(Container::LastResultNodePosition);
            }

            if (!NodeView::IsInsideRect(nodeView, view->visibleRect)) {
                ImVec2 defaultPosition = view->visibleRect.GetCenter();
                defaultPosition.x += view->visibleRect.GetWidth() * 1.0f / 6.0f;
                nodeView->setPosition(defaultPosition);
            }
        }
    }
    else
    {
        LOG_WARNING(0u, "Unable to restore result node position. resultNode is not set.\n" );
    }
}

size_t Container::getNodeCount()const
{
	return nodes.size();
}

Container::Container(const Language* _language)
{
	language = _language;
}
