#include "Node.h"
#include "Log.h"		// for LOG_DBG(...)
#include "Node_Container.h"
#include "Node_Variable.h"
#include "NodeView.h"

using namespace Nodable;

void Node::Connect(	Node* _from, 
					Node* _to, 
					const char* _fromOutputName, 
					const char* _toInputName)
{
	_from->setOutput (_to,   _fromOutputName);
	_to->setInput    (_from, _toInputName);
}

Node::Node()
{
	LOG_DBG("Node::Node()\n");
	this->inputs  = new Node_Container("inputs", this);
	this->outputs = new Node_Container("outputs", this);
	this->members = new Node_Container("members", this);
	this->view    = new NodeView(this);
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

Node_Container*   Node::getInputs      ()const
{
	return inputs;
}

Node_Variable* Node::getInput  (const char* _name)const
{
	return inputs->find(_name);
}

Node_Container*   Node::getOutputs      ()const
{
	return outputs;
}

Node_Variable* Node::getOutput (const char* _name)const
{
	return outputs->find(_name);
}

Node_Container*   Node::getMembers      ()const
{
	return members;
}

Node_Variable* Node::getMember (const char* _name)const
{
	return members->find(_name);
}

void Node::setInput  (Node* _node, const char* _name)
{
	inputs->setVariable(_name, _node);
}

void Node::setOutput (Node* _node, const char* _name)
{
	outputs->setVariable(_name, _node);
}

void Node::setMember (Node* _node, const char* _name)
{
	members->setVariable(_name, _node);
}

void Node::setLabel(const char* _label)
{
	this->label = _label;
}

void Node::setLabel(std::string _label)
{
	this->label = _label;
}

const char* Node::getLabel()const
{
	return this->label.c_str();
}

NodeView* Node::getView()const
{
	return this->view;
}