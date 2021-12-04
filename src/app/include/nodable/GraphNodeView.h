#pragma once

#include <map>
#include <string>
#include <functional>
#include <nodable/Reflect.h>

#include <nodable/AbstractCodeBlock.h>
#include <nodable/InvokableFunction.h>
#include <nodable/Nodable.h>     // forward declarations
#include <nodable/NodeView.h>     // base class

namespace Nodable
{
	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        const FunctionSignature*    function_signature;
	} FunctionMenuItem;

	class GraphNodeView: public NodeView
    {
	public:
	    GraphNodeView() = default;
		~GraphNodeView() = default;

		void    setOwner(Node*) override;
		void    updateViewConstraints();
		bool    draw() override ;
		bool    update() override;
		void    addContextualMenuItem(
		            const std::string&         _category,
		            const std::string&         _label,
		            std::function<Node*(void)> _lambda,
		            const FunctionSignature*   _signature);
	private:
	    std::vector<NodeViewConstraint> constraints;
        [[nodiscard]] GraphNode* getGraphNode() const;
		std::multimap<std::string, FunctionMenuItem> contextualMenus;

		REFLECT_DERIVED(GraphNodeView)
    REFLECT_EXTENDS(NodeView)
    REFLECT_END

    };
}