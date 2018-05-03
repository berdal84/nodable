#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include <algorithm>    // std::find_if
#include <cstring>      // for strcmp
#include "Node_Value.h"
#include "Node_Number.h"
#include "Node_String.h"
#include "Node_Lexer.h"
#include "Node_Container.h"
#include "Node_Variable.h"
#include "Node_BinaryOperations.h"

using namespace Nodable;

 // Node :
//////////

Node::Node()
{
	LOG_DBG("Node::Node()\n");
	this->inputs  = new Node_Container("inputs", this);
	this->outputs = new Node_Container("outputs", this);
	this->members = new Node_Container("members", this);
}

Node::~Node()
{
	delete inputs;
	delete outputs;
	delete members;
}

Node_Container* Node::getParent()const
{
	return this->parent;
}

void Node::setParent(Node_Container* _container)
{
	this->parent = _container;
}

Node_Variable* Node::getInput  (const char* _name)const
{
	return inputs->find(_name);
}

Node_Variable* Node::getOutput (const char* _name)const
{
	return outputs->find(_name);
}

Node_Variable* Node::getMember (const char* _name)const
{
	return members->find(_name);
}

void Node::setInput  (Node* _node, const char* _name)
{
	inputs->setSymbol(_name, _node);
}

void Node::setOutput (Node* _node, const char* _name)
{
	outputs->setSymbol(_name, _node);
}

void Node::setMember (Node* _node, const char* _name)
{
	members->setSymbol(_name, _node);
}

void Node::DrawRecursive(Node* _node, std::string _prefix)
{
	if ( _node == nullptr)
		return;	

	if(dynamic_cast<Node_BinaryOperation*>(_node))
	{
		_prefix = _prefix + "   ";
	}

	// Draw the node
	LOG_MSG("%s", _prefix.c_str());
	_node->draw();
	LOG_MSG("\n");

	// Draw its inputs
	for(auto each : _node->inputs->getVariables())
	{
		DrawRecursive(each->getValueAsNode(), _prefix);
	}
}