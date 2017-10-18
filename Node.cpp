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
	//cout <<  "New Node_Integer : " << value << endl;
}

Node_Integer::Node_Integer(std::string _string)
{
	value = std::stoi(_string);
	//cout <<  "New Node_Integer : " << value << endl;
}

Node_Integer::~Node_Integer(){}

void Node_Integer::setValue(int _n)
{
	//cout <<  "Node_Integer " <<  this->value << " becomes " << _n << endl;
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
	//cout <<  "New Node_String : " << _value << endl;
}

Node_String::~Node_String(){}

void Node_String::setValue(const char* _value)
{
	//cout <<  "Node_String " <<  this->value << " becomes " << _value << endl;
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
	//cout << "New Node_Tag : " << _name << endl;
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

 // Node_Lexer :
////////////////

Node_Lexer::Node_Lexer(Node_String* _expression):
expression(_expression)
{
	printf("New Node_Lexer ready to tokenize \"%s\"\n", _expression->getValue() );
}

Node_Lexer::~Node_Lexer()
{

}

void Node_Lexer::evaluate()
{
	tokenize();
	if ( isSyntaxValid() )
	{
		buildExecutionTreeAndEvaluate();
	}
}

void Node_Lexer::buildExecutionTreeAndEvaluateRec(size_t _tokenIndex, Node_Integer* _finalRes, Node_Integer* _prevRes)
{

	Node_Integer* 	left;
	Node_Integer* 	right;

	printf("Token evaluated : %lu.\n", _tokenIndex);

	// If a previous result is set, it mean we have already calculated the left part
	// So we use it as left operand

	if ( _prevRes != nullptr ){
		left = _prevRes;

	// Else, we parse the left operand
	}else{
		left = new Node_Integer(tokens[_tokenIndex].second.c_str());
	}
	

	size_t tokenLeft = tokens.size() - _tokenIndex;
	/* number */
	if ( tokenLeft == 1)
	{
		_finalRes->setValue(left->getValue());

	/* number, op, expr */
	}else if  (tokenLeft == 3){
		right 	= new Node_Integer(tokens[_tokenIndex+2].second.c_str());
		buildExecutionTreeAndEvaluateRec(_tokenIndex+2, right, nullptr);		
		Node_Add*		op 		= new Node_Add(left, right, _finalRes);	
		op->evaluate();

	/* number, op, number, op, expr */
	}else if  (tokenLeft >= 4)
	{	
		std::string currOperator = tokens[_tokenIndex+1].second;		
		right 	= new Node_Integer(tokens[_tokenIndex+2].second.c_str());
		std::string nextOperator = tokens[_tokenIndex+3].second;

		/* if currOperator is more important than nextOperator
		   we perform the first operation and send the result as left operand to the next expression */
		bool evaluateNow =  currOperator == nextOperator;
		if ( evaluateNow ){
			// Perform the operation on the left
			Node_Integer* 	result 	= new Node_Integer();			
			Node_Add*		op 		= new Node_Add(left, right, result);
			op->evaluate();
			// Pass the result and build the next operations
			buildExecutionTreeAndEvaluateRec(_tokenIndex+2, _finalRes, result);	

		/* Else, we evaluate the next expression and then perform the operation with 
		the result of the next expresssion as right operand */
		}else{
			buildExecutionTreeAndEvaluateRec(_tokenIndex+2, right, nullptr);		
			Node_Add*		op 		= new Node_Add(left, right, _finalRes);	
			op->evaluate();
		}
	}
}


void Node_Lexer::buildExecutionTreeAndEvaluate()
{
	printf("Node_Lexer::buildExecutionTreeAndEvaluate() - START\n");
	auto currentTokenIndex = 0;
	Node_Integer* result = new Node_Integer();	
	buildExecutionTreeAndEvaluateRec(currentTokenIndex, result, nullptr);
	printf("Node_Lexer::buildExecutionTreeAndEvaluate() - DONE !\n");
	cout << "Result: " << result->getValue() << endl;
}

bool Node_Lexer::isSyntaxValid()
{
	bool success = true;	
	printf("Node_Lexer::isSyntaxValid() - START\n");


	if(!( tokens.size()%2 == 1))
	{
		for(size_t i = 0; i < tokens.size(); i=i+2){
			if ( tokens[i].first != "number")
				success = false;
		}
		for(size_t i = 1; i < tokens.size(); i=i+2){
			if ( tokens[i].first != "operator")
				success = false;
		}
		printf("The only syntax accepted is \"number\", \"operator\", \"number\", etc... \n");
		success = false;
	}

	if(success)
		printf("Node_Lexer::isSyntaxValid() - OK\n");
	else
		printf("Node_Lexer::isSyntaxValid() - FAIL...\n");

	return success;
}

void Node_Lexer::tokenize()
{
	printf("Node_Lexer::tokenize() - START\n");
	/* get expression chars */
	std::string chars = expression->getValue();

	/* prepare allowed chars */
	string numbers 		= "0123456789.";
	string letters		= "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
	string operators 	= "+";

	for(auto it = chars.begin(); it < chars.end(); ++it)
	{

		 /* Search for a number */
		/////////////////////////

				if( numbers.find(*it) != string::npos )
		{

			auto itStart = it;
			while(	it != chars.end() && 
					numbers.find(*it) != string::npos)
			{
				++it;
			}
			--it;

			std::string number = chars.substr(itStart - chars.begin(), it - itStart + 1);
			addToken("number", number);

		 /* Search for a string */
		/////////////////////////

		}else 	if(*it == '"')
		{
			++it;
			auto itStart = it;
			while(	it != chars.end() && *it != '"')
			{
				++it;
			}

			std::string str = chars.substr(itStart - chars.begin(), it - itStart);
			addToken("string", str);

		 /* Search for a symbol */
		/////////////////////////

		}else 	if( letters.find(*it) != string::npos)
		{
			auto itStart = it;
			while(	it != chars.end() && 
					letters.find(*it) != string::npos)
			{
				++it;
			}
			--it;

			std::string str = chars.substr(itStart - chars.begin(), it - itStart + 1);
			addToken("symbol", str);

		 /* Search for an operator */
		////////////////////////////
			
		}else 	if(operators.find(*it) != string::npos)
		{
			std::string str = chars.substr(it - chars.begin(), 1);
			addToken("operator", str);
		}		
	}
	printf("Node_Lexer::tokenize() - DONE !\n");
}

void Node_Lexer::addToken(string _category, string _string)
{
	std::pair<std::string,std::string> token (_category, _string);
	printf("Node_Lexer::addToken() - %-10s => \"%s\" \n", ("\"" + _category + "\"").c_str(), _string.c_str() );
	tokens.push_back(token);
}