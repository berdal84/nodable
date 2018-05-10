#include "Node_Variable.h"
#include "Node_String.h"
#include "Node_Number.h"
#include "Log.h"

using namespace Nodable;

Node_Variable::Node_Variable(const char* _name, Node* _target):
	Node_Value(Type_Variable)
{
	LOG_DBG("New Node_Variable : %s\n", _name);
	setName(_name);
	setValue(_target);	
}

Node_Variable::~Node_Variable()
{

}

Node* Node_Variable::getValueAsNode  ()
{
	return this->target;
}

double Node_Variable::getValueAsNumber()const
{
	if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		return asNodeValue->getValueAsNumber();
	return this->target == nullptr ? double(0) : double(1);
}

std::string Node_Variable::getValueAsString()const
{
	if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		return asNodeValue->getValueAsString();
	return this->target == nullptr ? "NULL" : "Node";
}

void Node_Variable::setName(const char* _name)
{
	name = _name;
}
void Node_Variable::setValue(Node* _node)
{
	this->target = _node;
	updateLabel();
}

void Node_Variable::setValue(const char* _value)
{
	if(this->target == nullptr)
		this->target = new Node_String(_value);
	else if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		asNodeValue->setValue(_value);
	updateLabel();
}

void Node_Variable::setValue(double _value)
{
	if(this->target == nullptr)
		this->target = new Node_Number(_value);
	else if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		asNodeValue->setValue(_value);
	updateLabel();
}

void Node_Variable::updateLabel()
{
	if (target == nullptr)
		setLabel(getName() + std::string(" : NULL"));
	else if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		setLabel(getName() + std::string(" : ") + asNodeValue->getValueAsString());
}

const char* Node_Variable::getName()const
{
	return name.c_str();
}