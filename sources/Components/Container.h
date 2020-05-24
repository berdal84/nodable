#pragma once

#include "Nodable.h"
#include "Component.h"
#include <string>
#include <vector>
#include <imgui/imgui.h>   // for ImVec2
#include <mirror.h>
#include "Language.h"

namespace Nodable{

	class Container: public Component{
	public:

		Container(const Language* _language);
		virtual ~Container();
		bool                      	update();
		void                      	clear();		
		Variable* 	          		findVariable(std::string);
		void                      	add(Node*);
		void                      	remove(Node*);
		size_t                    	getNodeCount()const;
		std::vector<Variable*>& 	getVariables(){return variables;}
		std::vector<Node*>& 	    getEntities(){return nodes;}
		Variable*                   getResultVariable(){ return result;}
		void                        tryToRestoreResultNodePosition();
		
		/* node factory */
		Variable*					newResult();
		Variable*					newVariable(std::string = "");
		Variable*					newNumber(double = 0);
		Variable*					newNumber(const char*);
		Variable*					newString(const char*);	
		Node*                       newBinOp(std::string, const FunctionPrototype _proto);
		Parser*                    	newParser(Variable*);
		Wire*                       newWire();
		Node*                       newFunction(const FunctionPrototype& _proto);

	private:		
		Variable*                   result = nullptr;
		std::vector<Variable*> 		variables; /* Contain all Symbol Nodes created by this context */
		std::vector<Node*>          nodes;   /* Contain all Objects created by this context */
		const Language*             language;
	public:
		static ImVec2               LastResultNodePosition;

		MIRROR_CLASS(Container)(
			MIRROR_PARENT(Component));
	};
}