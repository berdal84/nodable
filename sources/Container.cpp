#include "Container.h"
#include "Log.h"
#include "Lexer.h"
#include "Entity.h"
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

Container::~Container()
{
	clear();
}

void Container::clear()
{
	for (Entity* each : entities)
		delete each;
	
	entities.resize(0);
	variables.resize(0);
}

void Container::update()
{
	// Update entities
	size_t entitiesUpdated(0);

	for(auto it = entities.begin(); it < entities.end(); ++it)
	{
		if ( *it != nullptr)
		{
			if ((*it)->needsToBeDeleted())
			{
				delete *it;
				it = entities.erase(it);
			}
			else if ((*it)->isDirty())
			{
				entitiesUpdated++;
				(*it)->update();
			}
		}
	}

	// Update TextEditor only if at least one node was dirty and one node is selected
	if (entitiesUpdated > 0 && NodeView::GetSelected() != nullptr)
	{
		auto app = this->getOwner()->getAs<Application*>();
		if (result && app)
		{
			app->updateCurrentLineText(result->getValueMember()->getSourceExpression());
		}
	}
}

void Container::addEntity(Entity* _entity)
{
	/* Add the node to the node vector list */
	this->entities.push_back(_entity);

	/* Set the node's container to this */
	_entity->setParent(this);

	LOG_DBG("A node has been added to the container '%s'\n", this->getMember("name")->getValueAsString().c_str());
}

void Container::destroyNode(Entity* _entity)
{
	{
		auto it = std::find(variables.begin(), variables.end(), _entity);
		if (it != variables.end())
			variables.erase(it);
	}

	{
		auto it = std::find(entities.begin(), entities.end(), _entity);
		if (it != entities.end())
			entities.erase(it);
	}

	delete _entity;
}

Variable* Container::find(std::string _name)
{
	Variable* result = nullptr;

	LOG_DBG("Searching node '%s' in container '%s' : ", _name.c_str(), this->getMember("name")->getValueAsString().c_str());

	auto findFunction = [_name](const Variable* _variable ) -> bool
	{
		return strcmp(_variable->getName(), _name.c_str()) == 0;
	};

	auto it = std::find_if(variables.begin(), variables.end(), findFunction);
	if (it != variables.end()){
		LOG_DBG("FOUND !\n");
		result = *it;
	}

	return result;
}

Variable* Container::createNodeResult()
{
	auto variable = createNodeVariable(ICON_FA_SIGN_OUT_ALT);
	auto member = variable->getMember("value");
	member->setConnectionFlags(Connection_In);                     // disable output because THIS node is the output !
	result = variable;
	return variable;
}

Variable* Container::createNodeVariable(std::string _name)
{
	auto variable = new Variable();
	variable->addComponent( "view", new NodeView);
	variable->setName(_name.c_str());
	this->variables.push_back(variable);
	this->addEntity(variable);
	return variable;
}

Variable* Container::createNodeNumber(double _value)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setValue(_value);
	this->addEntity(node);
	return node;
}

Variable* Container::createNodeNumber(const char* _value)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setValue(std::stod(_value));
	this->addEntity(node);
	return node;
}

Variable* Container::createNodeString(const char* _value)
{
	auto node = new Variable();
	node->addComponent( "view", new NodeView);
	node->setValue(_value);
	this->addEntity(node);
	return node;
}


Entity* Container::createNodeBinaryOperation(std::string _op)
{
	Entity* node;

	if (_op == "+")
		node = createNodeAdd();
	else if (_op == "-")
		node = createNodeSubstract();
	else if (_op == "*")
		node = createNodeMultiply();
	else if (_op == "/")
		node = createNodeDivide();
	else
		node = nullptr;
	
	return node;
}


Entity* Container::createNodeAdd()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Entity();	
	node->setLabel(ICON_FA_PLUS);
	node->addMember("left",   Visibility_Default, Type_Number, Connection_In);
	node->addMember("right",  Visibility_Default, Type_Number, Connection_In);
	node->addMember("result", Visibility_Default, Type_Number, Connection_Out);
	
	// Create a binary operation component and link values.
	auto operation 	= new Add();
	operation->setLeft  (node->getMember("left"));
	operation->setRight (node->getMember("right"));
	operation->setResult(node->getMember("result"));
	operation->setOperatorAsString("+");
	node->addComponent( "operation", operation);
	
	// Create a view component
	node->addComponent( "view", new NodeView);

	this->addEntity(node);

	return node;
}

Entity* Container::createNodeSubstract()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Entity();	
	node->setLabel(ICON_FA_MINUS);
	node->addMember("left",   Visibility_Default, Type_Number, Connection_In);
	node->addMember("right",  Visibility_Default, Type_Number, Connection_In);
	node->addMember("result", Visibility_Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Substract();
	operation->setLeft  (node->getMember("left"));
	operation->setRight (node->getMember("right"));
	operation->setResult(node->getMember("result"));
	operation->setOperatorAsString("-");
	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->addEntity(node);

	return node;
}

Entity* Container::createNodeMultiply()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Entity();	
	node->setLabel(ICON_FA_TIMES);
	node->addMember("left",   Visibility_Default, Type_Number, Connection_In);
	node->addMember("right",  Visibility_Default, Type_Number, Connection_In);
	node->addMember("result", Visibility_Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Multiply();
	operation->setLeft  (node->getMember("left"));
	operation->setRight (node->getMember("right"));
	operation->setResult(node->getMember("result"));
	operation->setOperatorAsString("*");

	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->addEntity(node);

	return node;
}

Entity* Container::createNodeDivide()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Entity();	
	node->setLabel(ICON_FA_DIVIDE);
	node->addMember("left",   Visibility_Default, Type_Number, Connection_In);
	node->addMember("right",  Visibility_Default, Type_Number, Connection_In);
	node->addMember("result", Visibility_Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Divide();
	operation->setLeft  (node->getMember("left"));
	operation->setRight (node->getMember("right"));
	operation->setResult(node->getMember("result"));
	operation->setOperatorAsString("/");

	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->addEntity(node);

	return node;
}

Entity* Container::createNodeAssign()
{
	// Create a node with 2 inputs and 1 output
	auto node 		= new Entity();	
	node->setLabel("ASSIGN");
	node->addMember("left",   Visibility_Default, Type_Number, Connection_In);
	node->addMember("right",  Visibility_Default, Type_Number, Connection_In);
	node->addMember("result", Visibility_Default, Type_Number, Connection_Out);

	// Create a binary operation component and link values.
	auto operation 	= new Assign();
	operation->setLeft  (node->getMember("left"));
	operation->setRight (node->getMember("right"));
	operation->setResult(node->getMember("result"));
	operation->setOperatorAsString("=");

	node->addComponent( "operation", operation);

	// Create a view component
	node->addComponent( "view", new NodeView);

	this->addEntity(node);

	return node;
}

Wire* Container::createWire()
{
	Wire* wire = new Wire;
	wire->addComponent("view", new WireView);	
	this->addEntity(wire);
	return wire;
}

Lexer* Container::createNodeLexer(Variable* _expressionVariable)
{
	// Create a Lexer Node
	Lexer* node = new Lexer();
	node->setLabel(ICON_FA_COGS " Lexer");
	
	// Attach a NodeView on it
	auto view = new NodeView;
	view->setVisible(false);
	node->addComponent( "view", view);	

	// Link the _expressionVariable output with the Lexer's member "expression"
	auto wire = this->createWire();
	Entity::Connect(wire,_expressionVariable->getValueMember(), node->getMember("expression"));

	// Transmit data from _expressionVariable to Lexer's member "expression"
	node->getMember("expression")->setValue(_expressionVariable->getValueMember());

	this->addEntity(node);
	return node;
}

size_t Container::getSize()const
{
	return entities.size();
}