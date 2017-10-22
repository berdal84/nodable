/*

Author: Bérenger Dalle-Cort, 2017

ChangeLog :

v0.3:
	- Node_Context : is now used as a factory.
	- Node : each node can get its contexts with Node::getContext()
	- Added a change log.
	- Added version number into the header file (NODABLE_VERSION_MAJOR, NODABLE_VERSION_MINOR, NODABLE_VERSION)

v0.2:
	- New Binary Operations : Node_Substract, Node_Multiply, Node_Divide
	- Node_Lexer : nos supports operator precedence.

v0.1:
	- Node_Add : to add two Node_Numbers
	- Node_Lexer : first version able to evaluate additions.
*/

#pragma once
#define NODABLE_VERSION_MAJOR "0"
#define NODABLE_VERSION_MINOR "3"
#define NODABLE_VERSION NODABLE_VERSION_MAJOR "." NODABLE_VERSION_MINOR

#include "vector"
#include "string.h"		// for memcpy
#include "stdlib.h"		// for size_t
#include "iostream"

namespace Nodable{

	/* Forward declarations */
	class Node;
	class Node_Number;
	class Node_Add;
	class Node_Symbol;
	class Node_Context;
	class Node_String;
	class Node_Lexer;
	class Node_BinaryOperation;
	class Node_Substraction;

	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		~Node();
		Node_Context*     getContext()const;
		void              setContext(Node_Context* _context);
	private:
		Node_Context* context; /* the context that create this node */
	};

	/*
	Class operand is the base class for everything that can be evaluated
	
	An operand is oftend connected to Operations:
	   - as an input if it is a result
	   - as an output if it is an operand)
	*/
	template<typename T>
	class Node_Value : public Node{
	public:
		Node_Value(T _value):value(_value){};
		~Node_Value(){};
		void setValue(T _value){value = _value;};
		T getValue()const{return value;};		
	private:
		T value;
	};

	class Node_Number : public Node_Value<double>{
	public:
		~Node_Number();
		Node_Number();
		Node_Number(int _n);
		Node_Number(std::string _string);
	};

	class Node_String : public Node{
	public:
		Node_String(const char* _value="");
		~Node_String();
		void setValue(const char* /*value*/);
		const char* getValue()const;
	private:
		std::string value;
	};

	/* Node_BinaryOperation is an interface for all binary operations */
	class Node_BinaryOperation: public Node{
	public:		
		enum Operator{
			Operator_Add,
			Operator_Mul,
			Operator_COUNT,
		};
		Node_BinaryOperation(Node_Number* _leftInput, Node_Number* _rightInput, Node_Number* _output);
		virtual ~Node_BinaryOperation();
		virtual void                  evaluate               () = 0;
		/* return true is op needs to be evaluated before nextOp */
		static  bool                  NeedsToBeEvaluatedFirst(const char op, const char nextOp);
	protected:
		Node_Number* getLeftInput  ()const;
		Node_Number* getRightInput ()const;
		Node_Number* getOutput     ()const;
	private:
		Node_Number* leftInput;
		Node_Number* rightInput;
		Node_Number* output;
	};

	/* Implementation of the Node_BinaryOperation as a Sum */
	class Node_Add : public Node_BinaryOperation{
	public:
		Node_Add(Node_Number* _leftInput, Node_Number* _rightInput, Node_Number* _output);
		~Node_Add();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as a Substraction */
	class Node_Substract : public Node_BinaryOperation{
	public:
		Node_Substract(Node_Number* _leftInput, Node_Number* _rightInput, Node_Number* _output);
		~Node_Substract();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as a Multiplication */
	class Node_Multiply : public Node_BinaryOperation{
	public:
		Node_Multiply(Node_Number* _leftInput, Node_Number* _rightInput, Node_Number* _output);
		~Node_Multiply();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as a Division */
	class Node_Divide : public Node_BinaryOperation{
	public:
		Node_Divide(Node_Number* _leftInput, Node_Number* _rightInput, Node_Number* _output);
		~Node_Divide();
		void evaluate();
	};

	/* Node_Symbol is a node that identify a value with its name */
	class Node_Symbol : public Node{
	public:
		Node_Symbol(const char* _name, Node* _value);
		~Node_Symbol();
		Node* 			getValue()const;
		const char* 	getName()const;
	private:
		std::string 	name;
		Node* 			value;
		Node_Context* 	context;
	};

	/* Class Node_Context is a factory able to create all kind of Node 
	   All Symbol nodes's pointers created within this context are referenced in a vector to be found later */
	class Node_Context : public Node {
	public:
		Node_Context(const char* /*name*/);
		~Node_Context();
		Node_Symbol* 	          find                      (const char* /*Symbol name*/);
		void                      addNode                   (Node* /*Node to add to this context*/);
		Node_Symbol*              createNodeSymbol          (const char* /*name*/, Node_Number* /*value*/);
		Node_Number*              createNodeNumber          (int /*value*/);
		Node_Number*              createNodeNumber          (const char* /*value*/);
		Node_String*              createNodeString          (const char* /*value*/);
		Node_Add*                 createNodeAdd             (Node_Number* /*inputA*/, Node_Number*/*inputB*/, Node_Number*/*output*/);
		Node_Substract*           createNodeSubstract       (Node_Number* /*inputA*/, Node_Number*/*inputB*/, Node_Number*/*output*/);
		Node_Multiply*			  createNodeMultiply        (Node_Number* /*inputA*/, Node_Number*/*inputB*/, Node_Number*/*output*/);
		Node_Divide*			  createNodeDivide          (Node_Number* /*inputA*/, Node_Number*/*inputB*/, Node_Number*/*output*/); 
		Node_BinaryOperation*     createNodeBinaryOperation (const char, Node_Number* /*inputA*/, Node_Number*/*inputB*/, Node_Number*/*output*/);
		Node_Lexer*               createNodeLexer           (Node_String* /*expression*/);
	private:		
		std::vector<Node_Symbol*> symbols; /* Contain all Symbol Nodes created by this context */
		std::vector<Node*>        nodes;   /* Contain all Nodes created by this context */
		std::string 	          name;    /* The name of this context */
	};

	typedef std::pair<std::string, std::string> Token;

	class Node_Lexer : public Node
	{
	public:
		Node_Lexer(Node_String* _expression);
		~Node_Lexer();
		void evaluate			();
	private:
		void buildExecutionTreeAndEvaluateRec(size_t _tokenIndex, Node_Number* _finalRes, Node_Number* _prevRes = nullptr);
		void tokenize			();
		bool isSyntaxValid		();
		void buildExecutionTreeAndEvaluate	();
		void addToken			(std::string _category, std::string _string);
		Node_String*       expression;
		std::vector<Token> tokens;
	};
}