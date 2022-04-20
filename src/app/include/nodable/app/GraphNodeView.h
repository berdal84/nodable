#pragma once

#include <map>
#include <string>
#include <functional>
#include <nodable/core/reflection/type.>

#include <nodable/core/IScope.h>
#include <nodable/core/reflection/invokable.h>
#include <nodable/app/types.h>     // forward declarations
#include <nodable/app/View.h>  // base class
#include <nodable/core/Component.h>  // base class
#include <nodable/app/NodeView.h> // for NodeViewConstraint

#include <nodable/app/IAppCtx.h>

namespace Nodable
{
	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        const func_type*    function_signature;
	} FunctionMenuItem;

	class GraphNodeView: public View, public Component
    {
	public:
	    GraphNodeView(IAppCtx& _ctx): View(_ctx) {};
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
                        const func_type *_signature);
	private:
        [[nodiscard]] GraphNode* get_graph_node() const;
        std::vector<ViewConstraint>              m_child_view_constraints;
		std::multimap<std::string, FunctionMenuItem> m_contextual_menus;

        static constexpr const char* k_context_menu_popup = "GraphNodeView.ContextMenu";
        static constexpr const char* k_operator_menu_label = "Operators";
        static constexpr const char* k_function_menu_label = "Functions";

		REFLECT_DERIVED_CLASS()

    };
}