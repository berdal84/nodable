#pragma once

#include <map>
#include <string>
#include <functional>
#include <nodable/Reflect.h>

#include <nodable/AbstractCodeBlock.h>
#include <nodable/InvokableFunction.h>
#include <nodable/Nodable.h>     // forward declarations
#include <nodable/View.h>  // base class
#include <nodable/Component.h>  // base class
#include <nodable/NodeView.h> // for NodeViewConstraint

namespace Nodable
{
	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        const FunctionSignature*    function_signature;
	} FunctionMenuItem;

	class GraphNodeView: public View, public Component
    {
	public:
	    GraphNodeView() = default;
		~GraphNodeView() override = default;

        void        set_owner(Node *) override;
        bool        draw() override ;
        bool        update() override;
        void        update_child_view_constraints();
        inline void clear_child_view_constraints() { m_child_view_constraints.clear(); };
		void        add_contextual_menu_item(
                        const std::string &_category,
                        const std::string &_label,
                        std::function<Node *(void)> _lambda,
                        const FunctionSignature *_signature);
	private:
        [[nodiscard]] GraphNode* get_graph_node() const;
        std::vector<NodeViewConstraint>              m_child_view_constraints;
		std::multimap<std::string, FunctionMenuItem> m_contextual_menus;

		REFLECT_DERIVED(GraphNodeView)
        REFLECT_EXTENDS(View)
        REFLECT_EXTENDS(Component)
        REFLECT_END

    };
}