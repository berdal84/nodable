#pragma once

#include "Nodable.h"   // for constants and forward declarations
#include "Node.h"      // base class

namespace Nodable{
	/* Node_BinaryOperation is an interface for all binary operations */
	class Node_BinaryOperation: public Node{
	public:		
		Node_BinaryOperation();
		virtual ~Node_BinaryOperation(){};
		bool                          couldBeEvaluated       ();
		virtual bool                  eval()=0;
		/* return true is op needs to be evaluated before nextOp */
		static  bool                  NeedsToBeEvaluatedFirst(std::string op, std::string nextOp);
	};

	/* Implementation of the Node_BinaryOperation as a Sum */
	class Node_Add : public Node_BinaryOperation{
	public:
		Node_Add(){setLabel("ADD");};
		~Node_Add(){};
		bool eval();
	};

	/* Implementation of the Node_BinaryOperation as a Substraction */
	class Node_Substract : public Node_BinaryOperation{
	public:
		Node_Substract(){setLabel("SUBSTRACT");};
		~Node_Substract(){};
		bool eval();
	};

	/* Implementation of the Node_BinaryOperation as a Multiplication */
	class Node_Multiply : public Node_BinaryOperation{
	public:
		Node_Multiply(){setLabel("MULTIPLY");};
		~Node_Multiply(){};
		bool eval();
	};

	/* Implementation of the Node_BinaryOperation as a Division */
	class Node_Divide : public Node_BinaryOperation{
	public:
		Node_Divide(){setLabel("DIVIDE");};
		~Node_Divide(){};
		bool eval();
	};

	/* Implementation of the Node_BinaryOperation as an assignment */
	class Node_Assign : public Node_BinaryOperation{
	public:
		Node_Assign(){setLabel("ASSIGN");};
		~Node_Assign(){};
		bool eval();
	};
}
