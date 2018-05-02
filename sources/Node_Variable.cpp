#include "Node_Variable.h"
#include "Node_String.h"
#include "Node_Number.h"
#include "Log.h"

using namespace Nodable;

Node_Variable::Node_Variable(const char* _label, Node* _target):
	Node_Value(Type_Variable)
{
	LOG_DBG("New Node_Variable : %s\n", _label);
	this->target = _target;
	this->label = _label;
}

Node_Variable::~Node_Variable()
{

}

void Node_Variable::draw()
{
	LOG_MSG("[%s : %s]", getLabel().c_str(), getValueAsString().c_str());
}

Node* Node_Variable::getValueAsNode  ()
{
	return this->target;
}

double Node_Variable::getValueAsNumber()const
{
	if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		return asNodeValue->getValueAsNumber();
	return this->target == nullptr ? double{0} : double{1};
}

std::string Node_Variable::getValueAsString()const
{
	if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		return asNodeValue->getValueAsString();
	return this->target == nullptr ? "NULL" : "Node";
}

std::string Node_Variable::getLabel()const
{
	// If label is empty, we use its value as string.
	if ( this->label.size() == 0)
		return getValueAsString();
	return this->label;
}

void Node_Variable::setValue(Node* _node)
{
	this->target = _node;
}

void Node_Variable::setValue(const char* _value)
{
	if(this->target == nullptr)
		this->target = new Node_String(_value);
	else if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		asNodeValue->setValue(_value);
}

void Node_Variable::setValue(double _value)
{
	if(this->target == nullptr)
		this->target = new Node_Number(_value);
	else if(auto asNodeValue = dynamic_cast<Node_Value*>(this->target))
		asNodeValue->setValue(_value);
}