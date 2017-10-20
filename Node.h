#pragma once
#include "vector"
#include "string.h"		// for memcpy
#include "stdlib.h"		// for size_t
#include "iostream"

namespace Nodable{

	/* Forward declarations */
	class Node;
	class Node_Number;
	class Node_Add;
	class Node_Tag;
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
	};

	class Node_Number : public Node{
	public:
		Node_Number(double _n=0.0F);
		Node_Number(std::string _string);
		~Node_Number();
		void setValue(double _n);
		double getValue()const;
	private:
		double value;
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
		static  Node_BinaryOperation* Create                 (const char _operator, Node_Number* _leftInput, Node_Number* _rightInput, Node_Number* _output);
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

	class Node_Tag : public Node{
	public:
		Node_Tag(Node_Context* _context, const char* _name, Node* _value);
		~Node_Tag();
		Node* 			getValue()const;
		const char* 	getName()const;
	private:
		std::string 	name;
		Node* 			value;
		Node_Context* 	context;
	};

	class Node_Context : public Node {
	public:
		Node_Context(const char* /*name*/);
		~Node_Context();
		void 		add(Node_Tag*);
		Node_Tag* 	find(const char*);
	private:		
		std::vector<Node_Tag*> tags;
		std::string 	name;
	};

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
		Node_String* expression;
		std::vector<std::pair<std::string, std::string>> tokens;
	};
}