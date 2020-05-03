#include "Container.h"
#include "Log.h"
#include "Parser.h"
#include "Node.h"
#include "Variable.h"
#include "BinaryOperation.h"
#include "Wire.h"
#include "WireView.h"
#include "DataAccess.h"
#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if
#include "NodeView.h"
#include "Application.h"
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
	if (result != nullptr) {
		auto view = result->getComponent<NodeView>();
		Container::LastResultNodePosition = view->getRoundedPosition();
	}

	
	for (Node* each : nodes)
		delete each;
	
	nodes.resize(0);
	variables.resize(0);
	result = nullptr;
}

bool Container::update()
{
	// Update entities
	size_t updatedNodesCount(0);

	for(auto it = nodes.begin(); it < nodes.end(); ++it)
	{
		auto node = *it;

		if ( node )
		{
			if ( node->needsToBeDeleted())
			{
				delete node;
				it = nodes.erase(it);
			}
			else if ( node->isDirty())
			{
				updatedNodesCount++;
				node->update();
			}
		}
	}

	const bool hasChanged = updatedNodesCount > 0 && NodeView::GetSelected() != nullptr;

	return hasChanged;
}

void Container::add(Node* _node)
{
	this->nodes.push_back(_node);
	_node->setParent(this);
}

void Container::remove(Node* _entity)
{
	{
		auto it = std::find(variables.begin(), variables.end(), _entity);
		if (it != variables.end())
			variables.erase(it);
	}

	{
		auto it = std::find(nodes.begin(), nodes.end(), _entity);
		if (it != nodes.end())
			nodes.erase(it);
	}

	delete _entity;
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
	member->setConnectionFlags(Connection_In);                     // disable output because THIS node is the output !
	result = variable;

	return variable;
}

Variable* Container::newVariable(std::string _name)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setName(_name.c_str());
	this->variables.push_back(node);
	this->add(node);
	return node;
}

Variable* Container::newNumber(double _value)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setValue(_value);
	this->add(node);
	return node;
}

Variable* Container::newNumber(const char* _value)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setValue(std::stod(_value));
	this->add(node);
	return node;
}

Variable* Container::newString(const char* _value)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setValue(_value);
	this->add(node);
	return node;
}


Node* Container::newBinOp(std::string _op)
{
	Node* node;

	if (_op == "+")
		node = newAdd();
	else if (_op == "-")
		node = newSub();
	else if (_op == "*")
		node = newMult();
	else if (_op == "/")
		node = newDivide();
	else
		node = nullptr;
	
	return node;
}


Node* Container::newAdd()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Node();	
	node->setLabel(ICON_FA_PLUS " Add");
	node->add("left",   Default, Type_Number, Connection_In);
	node->add("right",  Default, Type_Number, Connection_In);
	node->add("result", Default, Type_Number, Connection_Out);
	
	// Create a binary operation component and link values.
	auto operation 	= new Add();
	operation->setLeft  (node->get("left"));
	operation->setRight (node->get("right"));
	operation->setResult(node->get("result"));
	operation->setOperatorAsString("+");
	node->addComponent( "operation", operation);
	
	// Create a view component
	node->addComponent( "view", new NodeView());

	this->add(node);

	return node;
}

Node* Container::newSub()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Node();	
	node->setLabel(ICON_FA_MINUS " Sub");
	node->add("left",   Default, Type_Number, Connection_In);
	node->add("right",  Default, Type_Number, Connection_In);
	node->add("result", Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Substract();
	operation->setLeft  (node->get("left"));
	operation->setRight (node->get("right"));
	operation->setResult(node->get("result"));
	operation->setOperatorAsString("-");
	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->add(node);

	return node;
}

Node* Container::newMult()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Node();	
	node->setLabel(ICON_FA_TIMES " Mult");
	node->add("left",   Default, Type_Number, Connection_In);
	node->add("right",  Default, Type_Number, Connection_In);
	node->add("result", Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Multiply();
	operation->setLeft  (node->get("left"));
	operation->setRight (node->get("right"));
	operation->setResult(node->get("result"));
	operation->setOperatorAsString("*");

	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->add(node);

	return node;
}

Node* Container::newDivide()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Node();	
	node->setLabel(ICON_FA_DIVIDE " Div");
	node->add("left",   Default, Type_Number, Connection_In);
	node->add("right",  Default, Type_Number, Connection_In);
	node->add("result", Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Divide();
	operation->setLeft  (node->get("left"));
	operation->setRight (node->get("right"));
	operation->setResult(node->get("result"));
	operation->setOperatorAsString("/");

	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->add(node);

	return node;
}

Node* Container::newAssign()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Node();	
	node->setLabel("ASSIGN");
	node->add("left",   Default, Type_Number, Connection_In);
	node->add("right",  Default, Type_Number, Connection_In);
	node->add("result", Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Assign();
	operation->setLeft  (node->get("left"));
	operation->setRight (node->get("right"));
	operation->setResult(node->get("result"));
	operation->setOperatorAsString("=");

	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->add(node);

	return node;
}

Wire* Container::newWire()
{
	Wire* wire = new Wire;
	wire->addComponent("view", new WireView);	
	this->add(wire);
	return wire;
}

void Container::tryToRestoreResultNodePosition()
{
	// Store the Result node position to restore it later
	auto nodeView = result->getComponent<NodeView>();	
	bool resultNodeHadPosition = Container::LastResultNodePosition.x != -1 &&
	                             Container::LastResultNodePosition.y != -1;

	if (nodeView && this->hasComponent<View>() ) {

		auto view = this->getComponent<View>();

		if ( resultNodeHadPosition) {                                 /* if result node had a position stored, we restore it */
			nodeView->setPosition(Container::LastResultNodePosition);			
		}

		if ( !NodeView::IsInsideRect(nodeView, view->visibleRect) ){     		
			ImVec2 defaultPosition = view->visibleRect.GetCenter();
			defaultPosition.x += view->visibleRect.GetWidth() * 1.0f / 6.0f;
			nodeView->setPosition(defaultPosition);
		}
	}
}

Parser* Container::newParser(Variable* _expressionVariable)
{
	// Create a Parser Node
	auto language = Language::NODABLE;
	Parser* node = new Parser( language );
	node->setLabel(ICON_FA_COGS " Parser");
	
	// Attach a NodeView on it
	auto view = new NodeView;
	view->setVisible(false);
	node->addComponent( "view", view);	

	// Link the _expressionVariable output with the Parser's member "expression"
	auto wire             = this->newWire();
	auto expressionMember = node->get("expression");
	Node::Connect(wire,_expressionVariable->getValue(), expressionMember);
	expressionMember->updateValueFromInputMemberValue();

	this->add(node);
	return node;
}

size_t Container::getNodeCount()const
{
	return nodes.size();
}