#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "Node.h"
#include "vector"
#include <string>
#include "stdlib.h"		// for size_t

namespace Nodable{
	/* Node_BinaryOperation is an interface for all binary operations */
	class Node_BinaryOperation: public Node{
	public:		
		Node_BinaryOperation(){};
		virtual ~Node_BinaryOperation(){};
		virtual void                  draw()override{printf("%s", "[BinaryOperation]");}
		virtual void                  evaluate               () = 0;
		/* return true is op needs to be evaluated before nextOp */
		static  bool                  NeedsToBeEvaluatedFirst(std::string op, std::string nextOp);
	protected:
		Node_Value* getLeftInputValue  ()const;
		Node_Value* getRightInputValue ()const;
		Node_Value* getOutputValue     ()const;
	};

	/* Implementation of the Node_BinaryOperation as a Sum */
	class Node_Add : public Node_BinaryOperation{
	public:
		Node_Add(){};
		~Node_Add(){};
		void evaluate();
		void draw()override{printf("%s", "[Add]");}
	};

	/* Implementation of the Node_BinaryOperation as a Substraction */
	class Node_Substract : public Node_BinaryOperation{
	public:
		Node_Substract(){};
		~Node_Substract(){};
		void evaluate();
		void draw()override{printf("%s", "[Substract]");}
	};

	/* Implementation of the Node_BinaryOperation as a Multiplication */
	class Node_Multiply : public Node_BinaryOperation{
	public:
		Node_Multiply(){};
		~Node_Multiply(){};
		void evaluate();
		void draw()override{printf("%s", "[Multiply]");}
	};

	/* Implementation of the Node_BinaryOperation as a Division */
	class Node_Divide : public Node_BinaryOperation{
	public:
		Node_Divide(){};
		~Node_Divide(){};
		void evaluate();
		void draw()override{printf("%s", "[Divide]");}
	};

	/* Implementation of the Node_BinaryOperation as an assignment */
	class Node_Assign : public Node_BinaryOperation{
	public:
		Node_Assign(){};
		~Node_Assign(){};
		void evaluate();
		void draw()override{printf("%s", "[Assign]");}
	};
}
