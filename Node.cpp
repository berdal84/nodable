#include "Node.h"
#include "iostream"

using namespace Nodable;
using namespace std;

 // Node :
//////////

Node::Node(){}
Node::~Node(){}

 // Node_Integer :
//////////////////

Node_Integer::Node_Integer(int _n):
value(_n){}

Node_Integer::~Node_Integer(){}

void Node_Integer::setValue(int _n)
{
	this->value = _n;
}

int Node_Integer::getValue()const
{
	return this->value;
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
	this->output->setValue(result);
}