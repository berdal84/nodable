#pragma once

#include "Nodable.h"     // forward declarations
#include "Node.h"        // base class
#include <string>
#include <vector>

namespace Nodable{
	/* Class Node_Container is a factory able to create all kind of Node 
	   All Symbol nodes's pointers created within this context are referenced in a vector to be found later */
	class Node_Container {
	public:
		Node_Container(const char* _name, Node* _parent = nullptr);
		virtual ~Node_Container();
		void                      draw();
		void                      clear();
		void                      frameAll();
		void                      drawLabelOnly();
		size_t                    getSize()const;
		Node_Variable* 	          find                      (const char* /*Symbol name*/);
		void                      addNode                   (Node* /*Node to add to this context*/);
		void                      destroyNode               (Node*);
		Node_Variable*            createNodeVariable        (const char* /*name*/ = "");
		Node_Variable*            createNodeNumber          (double /*value*/ = 0);
		Node_Variable*            createNodeNumber          (const char* /*value*/);
		Node_Variable*            createNodeString          (const char* /*value*/);
		Node*                     createNodeBinaryOperation (std::string /*_operator*/);
		Node*                     createNodeAdd             ();
		Node*                     createNodeSubstract       ();
		Node*			          createNodeMultiply        ();
		Node*			          createNodeDivide          ();
		Node*			          createNodeAssign          (); 
		Node_Lexer*               createNodeLexer           (Node_Variable* /*expression*/);

		const char* 	          getName                   ()const;
		std::vector<Node_Variable*>& getVariables(){return variables;}
	private:
		std::vector<Node_Variable*> variables; /* Contain all Symbol Nodes created by this context */
		std::vector<Node*>        nodes;   /* Contain all Nodes created by this context */
		std::string 	          name;    /* The name of this context */
		Node*                     parent;
	};
}