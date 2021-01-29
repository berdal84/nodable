#pragma once

#include <string>
#include <vector>
#include <imgui/imgui.h>   // for ImVec2
#include <mirror.h>
#include <Language/Common/CodeBlock.h>

#include "Nodable.h"
#include "Component.h"
#include "Node.h"
#include "Language/Common/Language.h"

namespace Nodable{

    // forward declaration
    class Scope;
    class ResultNode;

	class Container: public Node {
	public:

		Container(const Language* _language);
		virtual ~Container();
        UpdateResult                update() override;
		void                      	clear();		
		Variable* 	          		findVariable(std::string);
		void                      	add(Node*);
		void                      	remove(Node*);
		size_t                    	getNodeCount()const;
		std::vector<Variable*>& 	getVariables(){return variables;}
		std::vector<Node*>& 	    getEntities(){return nodes;}
		const std::vector<ResultNode*>& getResults(){ return results;}
		void                        arrangeResultNodeViews();
        const Language*             getLanguage()const;

		/* node factory */
		ResultNode*					newInstructionResult();
		Variable*					newVariable(std::string = "");
		Variable*					newNumber(double = 0);
		Variable*					newNumber(const char*);
		Variable*					newString(const char*);	
		Node*                       newBinOp(const Operator*);
		Node*                       newUnaryOp(const Operator*);
        Node*                       newOperator(const Operator*);
		Wire*                       newWire();
		Node*                       newFunction(const Function* _proto);

	private:		
		std::vector<ResultNode*>    results;
		std::vector<Variable*> 		variables; /* Contain all Symbol Nodes created by this context */
		std::vector<Node*>          nodes;   /* Contain all Objects created by this context */
		const Language*             language;
		Scope*                      scope;
	public:
		static ImVec2               LastResultNodeViewPosition;

		MIRROR_CLASS(Container)(
			MIRROR_PARENT(Component));

        Scope *getScope()
        {
            return scope;
        }
    };
}