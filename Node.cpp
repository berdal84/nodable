#include "Node.h"
#include "iostream"		// cout
#include <algorithm>    // std::find_if
#include <stdio.h>		// printf("%s\n", );

using namespace Nodable;
using namespace std;

 // Node :
//////////

Node::Node(){}
Node::~Node(){}

 // Node_Integer :
//////////////////

Node_Integer::Node_Integer(int _n):
value(_n)
{
	cout <<  "New Node_Integer : " << _n << endl;
}

Node_Integer::~Node_Integer(){}

void Node_Integer::setValue(int _n)
{
	cout <<  "Node_Integer " <<  this->value << " becomes " << _n << endl;
	this->value = _n;
}

int Node_Integer::getValue()const
{
	return this->value;
}

 // Node_String :
//////////////////

Node_String::Node_String(const char* _value):
value(_value)
{
	cout <<  "New Node_String : " << _value << endl;
}

Node_String::~Node_String(){}

void Node_String::setValue(const char* _value)
{
	cout <<  "Node_String " <<  this->value << " becomes " << _value << endl;
	this->value = _value;
}

const char* Node_String::getValue()const
{
	return this->value.c_str();
}

 // Node_Add :
//////////////

Node_Add::Node_Add(	Node_Integer* _inputA,
					Node_Integer* _inputB,
					Node_Integer* _output):
	inputA(_inputA),
	inputB(_inputB),
	output(_output)
{

}

Node_Add::~Node_Add()
{

}

void Node_Add::evaluate()
{
	int result = this->inputA->getValue() + this->inputB->getValue();
	cout <<  "Node_Add:evaluate(): " <<  this->inputA->getValue() << " + " << this->inputB->getValue() << " = " << result << endl;
	this->output->setValue(result);
}

 // Node_Tag :
//////////////

Node_Tag::Node_Tag(Node_Context* _context, const char* _name, Node* _value):
	name(_name),
	value(_value),
	context(_context)	
{
	cout << "New Node_Tag : " << _name << endl;
	_context->add(this);
}

Node_Tag::~Node_Tag()
{

}

const char* Node_Tag::getName()const
{
	return name.c_str();
}

Node* Node_Tag::getValue()const
{
	return this->value;
}

 // Node_Context :
//////////////////

Node_Context::Node_Context(const char* _name):
name(_name)
{

}

Node_Context::~Node_Context()
{

}

void Node_Context::add(Node_Tag* _node)
{
	tags.push_back(_node);
}

Node_Tag* Node_Context::find(const char* _name)
{
	printf("Searching node with name '%s' in context named '%s' : ", _name, this->name.c_str());

	auto findFunction = [_name](const Node_Tag* _node ) -> bool
	{
		return strcmp(_node->getName(), _name) == 0;
	};

	auto it = std::find_if(tags.begin(), tags.end(), findFunction);
	if (it != tags.end()){
		cout << "found." << endl;
		return *it;
	}
	cout << "NOT found !" << endl;
	return nullptr;
}