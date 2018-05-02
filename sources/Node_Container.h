#pragma once

#include "Nodable.h"     // forward declarations
#include "Node.h"        // base class
#include <string>
#include <vector>

namespace Nodable{
	/* Class Node_Container is a factory able to create all kind of Node 
	   All Symbol nodes's pointers created within this context are referenced in a vector to be found later */
	class Node_Container : public Node {
	public:
		Node_Container(const char* /*name*/);
		virtual ~Node_Container(){};
		Node_Symbol* 	          find                      (const char* /*Symbol name*/);
		void                      addNode                   (Node* /*Node to add to this context*/);
		Node_Symbol*              createNodeSymbol          (const char* /*name*/, Node_Value* /*value*/);
		Node_Number*              createNodeNumber          (int /*value*/ = 0);
		Node_Number*              createNodeNumber          (const char* /*value*/);
		Node_String*              createNodeString          (const char* /*value*/);
		Node_Add*                 createNodeAdd             (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Substract*           createNodeSubstract       (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Multiply*			  createNodeMultiply        (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Divide*			  createNodeDivide          (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Assign*			  createNodeAssign          (Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/); 
		Node_BinaryOperation*     createNodeBinaryOperation (const char, Node_Value* /*inputA*/, Node_Value*/*inputB*/, Node_Value*/*output*/);
		Node_Lexer*               createNodeLexer           (Node_String* /*expression*/);
		const char* 	          getName                   ()const;
	private:
		std::vector<Node_Symbol*> symbols; /* Contain all Symbol Nodes created by this context */
		std::vector<Node*>        nodes;   /* Contain all Nodes created by this context */
		std::string 	          name;    /* The name of this context */
	};
}