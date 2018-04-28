
#include "Nodable.h"            // for constants and forward declarations
#include "vector"
#include "string.h"		// for memcpy
#include "stdlib.h"		// for size_t
#include "iostream"

namespace Nodable{


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
	Class value is the base class for everything that can be evaluated
	
	An operand is oftend connected to Operations:
	   - as an input if it is a result
	   - as an output if it is an operand)
	*/

	enum Type_{
		Type_Number,
		Type_String,
		Type_COUNT
	};

	class Node_Value : public Node{
	public:
		Node_Value(Type_ _type);
		~Node_Value();
		Type_          getType()const;
		bool           isType(Type_ _type)const;
		virtual Node_Number*   asNumber();
		virtual Node_String*   asString();
	private:
		Type_ type;
	};

	class Node_Number : public Node_Value{
	public:
		~Node_Number();
		Node_Number(double _n);
		Node_Number(std::string _string);
		double getValue()const;
		void   setValue(double _value);
	private:
		double value;
	};

	class Node_String : public Node_Value{
	public:
		Node_String(const char* _value="");
		~Node_String();
		void        setValue(const char* /*value*/);
		const char* getValue()const;
	private:
		std::string value;
	};

	/* Node_BinaryOperation is an interface for all binary operations */
	class Node_BinaryOperation: public Node{
	public:		
		Node_BinaryOperation(Node_Value* _leftInput, Node_Value* _rightInput, Node_Value* _output);
		virtual ~Node_BinaryOperation();
		virtual void                  evaluate               () = 0;
		/* return true is op needs to be evaluated before nextOp */
		static  bool                  NeedsToBeEvaluatedFirst(const char op, const char nextOp);
	protected:
		Node_Value* getLeftInput  ()const;
		Node_Value* getRightInput ()const;
		Node_Value* getOutput     ()const;
	private:
		Node_Value* leftInput;
		Node_Value* rightInput;
		Node_Value* output;
	};

	/* Implementation of the Node_BinaryOperation as a Sum */
	class Node_Add : public Node_BinaryOperation{
	public:
		Node_Add(Node_Value* _leftInput, Node_Value* _rightInput, Node_Value* _output);
		~Node_Add();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as a Substraction */
	class Node_Substract : public Node_BinaryOperation{
	public:
		Node_Substract(Node_Value* _leftInput, Node_Value* _rightInput, Node_Value* _output);
		~Node_Substract();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as a Multiplication */
	class Node_Multiply : public Node_BinaryOperation{
	public:
		Node_Multiply(Node_Value* _leftInput, Node_Value* _rightInput, Node_Value* _output);
		~Node_Multiply();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as a Division */
	class Node_Divide : public Node_BinaryOperation{
	public:
		Node_Divide(Node_Value* _leftInput, Node_Value* _rightInput, Node_Value* _output);
		~Node_Divide();
		void evaluate();
	};

	/* Implementation of the Node_BinaryOperation as an assignment */
	class Node_Assign : public Node_BinaryOperation{
	public:
		Node_Assign(Node_Value* _leftInput, Node_Value* _rightInput, Node_Value* _output);
		~Node_Assign();
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
		Node_Symbol*              createNodeSymbol          (const char* /*name*/, Node_Value* /*value*/);
		Node_Number*              createNodeNumber          (int /*value*/);
		Node_Number*              createNodeNumber          (const char* /*value*/);
		Node_String*              createNodeString          (const char* /*value*/);
		Node_Add*                 createNodeAdd             (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Substract*           createNodeSubstract       (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Multiply*			  createNodeMultiply        (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Divide*			  createNodeDivide          (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Assign*			  createNodeAssign          (Node_Value* /*inputA*/, Node_Value*/*inpuNode_ValuetB*/, Node_Value*/*output*/); 
		Node_BinaryOperation*     createNodeBinaryOperation (const char, Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
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
		void           evaluate			                  ();
	private:
		void           buildExecutionTreeAndEvaluateRec   (size_t _tokenIndex, Node_Number* _finalRes, Node_Number* _prevRes = nullptr);
		void           tokenize			                  ();
		bool           isSyntaxValid		              ();
		void           buildExecutionTreeAndEvaluate      ();
		Node_Number*   convertTokenToNode                 (Token token);
		void           addToken			                  (std::string _category, std::string _string);

		Node_String*       expression;
		std::vector<Token> tokens;
	};
}
