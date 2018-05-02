#pragma once

#include "Nodable.h"            // for constants and forward declarations
#include "vector"
#include <string>
#include "stdlib.h"		// for size_t

#define NODE_DEFAULT_INPUT_NAME "default"
#define NODE_DEFAULT_OUTPUT_NAME "default"

namespace Nodable{


	/* Base class for all Nodes */
	class Node{
	public:
		Node();
		~Node();
		virtual void      draw           (){};
		Node_Container*   getParent      ()const;		
		Node_Variable*    getInput       (const char* _name = NODE_DEFAULT_INPUT_NAME)const;
		Node_Variable*    getOutput      (const char* _name = NODE_DEFAULT_OUTPUT_NAME)const;	
		Node_Variable*    getMember      (const char* _name)const;
		void              setInput       (Node*, const char* _name = NODE_DEFAULT_INPUT_NAME);
		void              setOutput      (Node*, const char* _name = NODE_DEFAULT_OUTPUT_NAME);
		void              setMember      (Node*, const char* _name);
		void              setParent      (Node_Container* _container);
		static void       DrawRecursive  (Node*, std::string _prefix = "");
	private:
		Node_Container* inputs;
		Node_Container* outputs;		
		Node_Container* members;
		Node_Container* parent = nullptr;
	};	

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
