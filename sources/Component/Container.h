#pragma once

#include <string>
#include <vector>
#include <memory>
#include <imgui/imgui.h>   // for ImVec2
#include <mirror.h>
#include <NodeFactory.h>
#include <Node.h>

namespace Nodable{

    class Variable;
    class Language;

	class Container: public Node {
	public:

		Container(const std::shared_ptr<const Language>& _language);
		virtual ~Container();
        UpdateResult                update() override;
		void                      	clear();		
		Variable* 	          		findVariable(std::string);
		void                      	add(const std::shared_ptr<Node>&);
		void                      	remove(const std::shared_ptr<Node>&);
		size_t                    	getNodeCount()const;
		std::map<std::string, Variable*>& 	getVariables(){return variables;}
		const std::vector<std::shared_ptr<Node>>& 	    getEntities(){return nodes;}
		Variable*                   getResultVariable(){ return resultNode.get();}
		void                        tryToRestoreResultNodePosition();
		
		/* node factory */
		std::shared_ptr<Variable>	newResult();
        std::shared_ptr<Variable>	newVariable(const std::string& = "");
        std::shared_ptr<Variable>   newNumber(double = 0);
        std::shared_ptr<Variable>	newNumber(const char*);
        std::shared_ptr<Variable>	newString(const char*);
        std::shared_ptr<Node>       newBinOp(std::shared_ptr<const Operator>);
        std::shared_ptr<Node>       newUnaryOp(std::shared_ptr<const Operator>);
        std::shared_ptr<Wire>       newWire();
        std::shared_ptr<Node>       newFunction(std::shared_ptr<const Function> _proto);

	private:
	    std::unique_ptr<NodeFactory> factory;
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