#include "Node_Container.h"
#include "Log.h"
#include "Node_Number.h"
#include "Node_String.h"
#include "Node_Lexer.h"
#include "Node.h"
#include <cstring>      // for strcmp
#include <algorithm>    // for std::find_if

using namespace Nodable;

Node_Container::Node_Container(const char* _name):
name(_name)
{
	LOG_DBG("A new context named '%s' has been created.\n", _name);
}


void Node_Container::addNode(Node* _node)
{
	/* Add the node to the node vector list*/
	this->nodes.push_back(_node);

	/* Set the node's context to this */
	_node->setContext(this);

	LOG_DBG("A node has been added to the context '%s'\n", this->getName());
}

Node_Symbol* Node_Container::find(const char* _name)
{
	LOG_DBG("Searching node '%s' in context '%s' : ", _name, this->getName());

	auto findFunction = [_name](const Node_Symbol* _node ) -> bool
	{
		return strcmp(_node->getName(), _name) == 0;
	};

	auto it = std::find_if(symbols.begin(), symbols.end(), findFunction);
	if (it != symbols.end()){
		LOG_DBG("FOUND !\n");
		return *it;
	}
	LOG_DBG("NOT found...\n");
	return nullptr;
}

Node_Symbol* Node_Container::createNodeSymbol(const char* _name, Node_Value* _value)
{
	Node_Symbol* node = new Node_Symbol(_name, _value);
	addNode(node);
	this->symbols.push_back(node);
	return node;
}

Node_Number*          Node_Container::createNodeNumber(int _value)
{
	Node_Number* node = new Node_Number(_value);
	addNode(node);
	return node;
}

Node_Number*          Node_Container::createNodeNumber(const char* _value)
{
	Node_Number* node = new Node_Number(_value);
	addNode(node);
	return node;
}

Node_String*          Node_Container::createNodeString(const char* _value)
{
	Node_String* node = new Node_String(_value);
	addNode(node);
	return node;
}

Node_Add* Node_Container::createNodeAdd(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Add*)this->createNodeBinaryOperation('+', _inputA, _inputB, _output );
}

Node_Substract* Node_Container::createNodeSubstract(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Substract*)this->createNodeBinaryOperation('-', _inputA, _inputB, _output );
}

Node_Multiply* Node_Container::createNodeMultiply(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Multiply*)this->createNodeBinaryOperation('*', _inputA, _inputB, _output );
}

Node_Divide* Node_Container::createNodeDivide(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Divide*)this->createNodeBinaryOperation('/', _inputA, _inputB, _output );
}

Node_Assign* Node_Container::createNodeAssign(Node_Value* _inputA, Node_Value* _inputB, Node_Value* _output)
{
	return (Node_Assign*)this->createNodeBinaryOperation('=', _inputA, _inputB, _output);
}


Node_Lexer* Node_Container::createNodeLexer           (Node_String* _input)
{
	Node_Lexer* lexer = new Node_Lexer(_input);
	addNode(lexer);
	return lexer;
}

Node_BinaryOperation* Node_Container::createNodeBinaryOperation(   
	                            const char _operator, 
								Node_Value* _leftInput, 
								Node_Value* _rightInput, 
								Node_Value* _output)
{
	Node_BinaryOperation* node = nullptr;

	if ( _operator == '+')
		node = new Node_Add(_leftInput, _rightInput, _output);
	else if ( _operator == '-')
		node = new Node_Substract(_leftInput, _rightInput, _output);
	else if ( _operator == '/')
		node = new Node_Divide(_leftInput, _rightInput, _output);
	else if ( _operator == '*')
		node = new Node_Multiply(_leftInput, _rightInput, _output);
	else if ( _operator == '=')
		node = new Node_Assign(_leftInput, _rightInput, _output);

	addNode(node);

	return node;
}

const char* Node_Container::getName()const
{
	return name.c_str();
}