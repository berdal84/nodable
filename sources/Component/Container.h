#pragma once

#include <string>
#include <vector>
#include <imgui/imgui.h>   // for ImVec2
#include <mirror.h>
#include "Node.h"
#include "Language.h"

namespace Nodable{

    class Variable;

	class Container: public Node {
	public:

		Container(std::shared_ptr<const Language> _language);
		virtual ~Container();
        UpdateResult                update() override;
		void                      	clear();		
		Variable* 	          		findVariable(std::string);
		void                      	add(std::shared_ptr<Node>);
		void                      	remove(const std::shared_ptr<Node>);
		size_t                    	getNodeCount()const;
		std::map<std::string, Variable*>& 	getVariables(){return variables;}
		const std::vector<std::shared_ptr<Node>>& 	    getEntities(){return nodes;}
		Variable*                   getResultVariable(){ return resultNode.get();}
		void                        tryToRestoreResultNodePosition();
		
		/* node factory */
		Variable*					newResult();
		std::shared_ptr<Variable>	newVariable(std::string = "");
		Variable*					newNumber(double = 0);
		Variable*					newNumber(const char*);
		Variable*					newString(const char*);	
		Node*                       newBinOp(std::shared_ptr<const Operator>);
		Node*                       newUnaryOp(std::shared_ptr<const Operator>);
		std::shared_ptr<Wire>       newWire();
		Node*                       newFunction(std::shared_ptr<const Function> _proto);

	private:		
		std::shared_ptr<Variable> resultNode;
		std::map<std::string, Variable*> variables; /* Contain all Symbol Nodes created by this context */
		std::vector<std::shared_ptr<Node>> nodes;   /* Contain all Objects created by this context */
        std::shared_ptr<const Language> language;
	public:
		static ImVec2               LastResultNodePosition;

		MIRROR_CLASS(Container)(
			MIRROR_PARENT(Component));
	};
}