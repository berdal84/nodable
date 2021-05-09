#pragma once

#include <map>
#include <string>
#include <functional>
#include <AbstractCodeBlockNode.h>
#include <Common/Function.h>

#include "mirror.h"
#include "Nodable.h"     // forward declarations
#include "Component/NodeView.h"   	 // base class

namespace Nodable{

	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        FunctionSignature           function_signature;
	} FunctionMenuItem;

	class GraphNodeView: public NodeView {
	public:
	    GraphNodeView() = default;
		~GraphNodeView() = default;

		void    setOwner(Node*) override;
		void    updateViewConstraints();
		bool    draw() override ;
		bool    update() override;
		void    addContextualMenuItem(const std::string& _category, std::string _label, std::function<Node*(void)> _lambda, const FunctionSignature& _signature);
	private:
	    std::vector<ViewConstraint> constraints;
        [[nodiscard]] GraphNode* getGraphNode() const;
		std::multimap<std::string, FunctionMenuItem> contextualMenus;

		MIRROR_CLASS(GraphNodeView)
		(
			MIRROR_PARENT(NodeView)
        );

        static void DrawCodeFlowLine(NodeView *startView, NodeView *endView, short _slotCount = 1, short _slotPosition = 0);
    };
}