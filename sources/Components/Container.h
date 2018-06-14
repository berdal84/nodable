#pragma once

#include "Nodable.h"     // forward declarations
#include <Component.h>   // base class
#include <string>
#include <vector>

namespace Nodable{
	/* Class Container is a factory able to create all kind of Node 
	   All Variables's pointers created within this context are referenced in a vector to be found later */
	class Container: public Component{
	public:
		COMPONENT_CONSTRUCTOR(Container);

		virtual ~Container();
		void                      	draw();
		void                      	clear();
		size_t                    	getSize()const;
		Variable* 	          		find                      (std::string /*Symbol name*/);
		void                      	addEntity                 (Entity* /* _entity*/);
		void                      	destroyNode               (Entity*);
		Variable*					createNodeResult          ();
		Variable*					createNodeVariable        (std::string /*name*/ = "");
		Variable*					createNodeNumber          (double /*value*/ = 0);
		Variable*					createNodeNumber          (const char* /*value*/);
		Variable*					createNodeString          (const char* /*value*/);
		Entity*                   	createNodeBinaryOperation (std::string /*_operator*/);
		Entity*                   	createNodeAdd();
		Entity*                   	createNodeSubstract();
		Entity*			          	createNodeMultiply();
		Entity*			          	createNodeDivide();
		Entity*			          	createNodeAssign(); 
		Lexer*                    	createNodeLexer           (Variable* /*expression*/);
		Wire*                       createWire();
		std::vector<Variable*>& 	getVariables(){return variables;}
	private:
		std::vector<Variable*> 		variables; /* Contain all Symbol Nodes created by this context */
		std::vector<Entity*>        entities;   /* Contain all Objects created by this context */
	};
}