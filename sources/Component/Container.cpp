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
	// Store the Result node position to restore it later
	if (resultNode != nullptr) {
		auto view = resultNode->getComponent<NodeView>();
		Container::LastResultNodePosition = view->getRoundedPosition();
	}

	LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "=================== Container::clear() ==================\n");

	auto nodeIndex = nodes.size();

	while ( nodeIndex > 0)
    {
        nodeIndex--;
	    auto node = nodes.at(nodeIndex);
        LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "remove and delete: %s \n", node->getLabel() );
        remove(node);
        delete node;
	}
    nodes.resize(0);

    LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "===================================================\n");


    resultNode = nullptr;
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
	    return UpdateResult::SuccessWithChanges;
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
    {
        auto it = std::find(variables.begin(), variables.end(), _node);
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
        this->resultNode = nullptr;
    }
}

Variable* Container::findVariable(std::string _name)
{
	Variable* result = nullptr;

	auto findFunction = [_name](const Variable* _variable ) -> bool
	{
		return strcmp(_variable->getName(), _name.c_str()) == 0;
	};

	auto it = std::find_if(variables.begin(), variables.end(), findFunction);
	if (it != variables.end()){
		result = *it;
	}

	return result;
}

Variable* Container::newResult()
{
	auto variable = newVariable(ICON_FA_SIGN_OUT_ALT " Result");
	auto member = variable->get("value");
	member->setConnectorWay(Way_In);                     // disable output because THIS node is the output !
	resultNode = variable;

	return variable;
}

Variable* Container::newVariable(std::string _name)
{
	auto node = new Variable();
	node->addComponent( new NodeView);
	node->setName(_name.c_str());
	this->variables.push_back(node);
	this->add(node);
	return node;
}

Variable* Container::newNumber(double _value)
{
	auto node = new Variable();
	node->addComponent( new NodeView);
	node->set(_value);
	this->add(node);
	return node;
}

Variable* Container::newNumber(const char* _value)
{
	auto node = new Variable();
	node->addComponent( new NodeView);
	node->set(std::stod(_value));
	this->add(node);
	return node;
}

Variable* Container::newString(const char* _value)
{
	auto node = new Variable();
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
	auto left   = node->add("lvalue", Visibility::Default, language->tokenTypeToType(args[0].type), Way_In);
	auto right  = node->add("rvalue", Visibility::Default, language->tokenTypeToType(args[1].type), Way_In);
	auto result = node->add("result", Visibility::Default, language->tokenTypeToType(signature.getType()), Way_Out);

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
	auto left = node->add("lvalue", Visibility::Default, language->tokenTypeToType(args[0].type), Way_In);
	auto result = node->add("result", Visibility::Default, language->tokenTypeToType(signature.getType()), Way_Out);

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
	node->add("result", Visibility::Default, language->tokenTypeToType(_function->signature.getType()), Way_Out);

	// Create ComputeBase binOpComponent and link values.
	auto functionComponent = new ComputeFunction(_function, language);
	functionComponent->setResult(node->get("result"));

	// Arguments
	auto args = _function->signature.getArgs();
	for (size_t argIndex = 0; argIndex < args.size(); argIndex++) {
		std::string memberName = args[argIndex].name;
		auto member = node->add(memberName.c_str(), Visibility::Default, language->tokenTypeToType(args[argIndex].type), Way_In); // create node input
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

void Container::tryToRestoreResultNodePosition()
{
	// Store the Result node position to restore it later
	auto nodeView = resultNode->getComponent<NodeView>();
	bool resultNodeHadPosition = Container::LastResultNodePosition.x != -1 &&
	                             Container::LastResultNodePosition.y != -1;

	if (nodeView && this->hasComponent<View>() ) {

		auto view = this->getComponent<View>();

		if ( resultNodeHadPosition) {                                 /* if result node had a position stored, we restore it */
			nodeView->setPosition(Container::LastResultNodePosition);			
		}

		auto rect = view->getVisibleRect();
		if ( !NodeView::IsInsideRect(nodeView, rect ) ){
			ImVec2 defaultPosition = rect.GetCenter();
			defaultPosition.x += rect.GetWidth() * 1.0f / 6.0f;
			nodeView->setPosition(defaultPosition);
		}
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

const Language *Container::getLanguage()const {
    return language;
}
