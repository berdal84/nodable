#pragma once

#include <string>
#include <vector>
#include <imgui/imgui.h>   // for ImVec2
#include <mirror.h>
#include <Node/AbstractCodeBlockNode.h>

#include "Nodable.h"
#include "Component.h"
#include "Node.h"
#include "Language/Common/Language.h"

namespace Nodable{

    // forward declaration
    class ScopedCodeBlockNode;
    class InstructionNode;
    class CodeBlockNode;

	class Container: public Node {
	public:

		Container(const Language* _language);
		virtual ~Container();
        UpdateResult                update() override;
		void                      	clear();		
		VariableNode* 	          	findVariable(std::string);
		void                      	add(Node*);
		void                      	remove(Node*);
		size_t                    	getNodeCount()const;
		std::vector<Node*>& 	    getEntities(){return nodes;}
		void                        arrangeResultNodeViews();
        const Language*             getLanguage()const;
        ScopedCodeBlockNode*            getScope(){ return scope;}

		/* node factory */
		CodeBlockNode*              newCodeBlock();
        InstructionNode*		    newInstruction(CodeBlockNode* _parentCodeBlock);
		VariableNode*				newVariable(std::string, ScopedCodeBlockNode*);
		VariableNode*				newNumber(double = 0);
		VariableNode*				newNumber(const char*);
		VariableNode*				newString(const char*);
		Node*                       newBinOp(const Operator*);
		Node*                       newUnaryOp(const Operator*);
        Node*                       newOperator(const Operator*);
		Wire*                       newWire();
		Node*                       newFunction(const Function* _proto);

	private:		
		std::vector<Node*>          nodes;
		const Language*             language;
		ScopedCodeBlockNode*            scope;
	public:
		static ImVec2               LastResultNodeViewPosition;

		MIRROR_CLASS(Container)(
			MIRROR_PARENT(Component));

        bool hasInstructions();
    };
}