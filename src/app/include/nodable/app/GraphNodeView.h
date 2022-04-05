#pragma once

#include <map>
#include <string>
#include <functional>
#include <nodable/core/reflection/R.h>

#include <nodable/core/IScope.h>
#include <nodable/core/InvokableFunction.h>
#include <nodable/app/types.h>     // forward declarations
#include <nodable/app/View.h>  // base class
#include <nodable/core/Component.h>  // base class
#include <nodable/app/NodeView.h> // for NodeViewConstraint

#include <nodable/app/AppContext.h>

namespace Nodable
{
	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        const FuncSig*    function_signature;
	} FunctionMenuItem;

	class GraphNodeView: public View, public Component
    {
	public:
	    GraphNodeView(AppContext* _ctx): View(_ctx), m_context(_ctx) {};
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
                        const FuncSig *_signature);
	private:
        [[nodiscard]] GraphNode* get_graph_node() const;
        std::vector<NodeViewConstraint>              m_child_view_constraints;
		std::multimap<std::string, FunctionMenuItem> m_contextual_menus;
        AppContext* m_context;
        static constexpr const char* k_context_menu_popup = "GraphNodeView.ContextMenu";

		R_DERIVED(GraphNodeView)
        R_EXTENDS(View)
        R_EXTENDS(Component)
        R_END

    };
}