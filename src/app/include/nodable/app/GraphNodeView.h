#pragma once

#include <map>
#include <string>
#include <functional>
#include <nodable/core/reflection/reflection>

#include <nodable/core/IScope.h>
#include <nodable/core/reflection/invokable.h>
#include <nodable/app/types.h>     // forward declarations
#include <nodable/app/View.h>  // base class
#include <nodable/core/Component.h>  // base class
#include <nodable/app/NodeView.h> // for NodeViewConstraint

#include <nodable/app/IAppCtx.h>

namespace ndbl
{
	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        const func_type*    function_signature;
	} FunctionMenuItem;

	class GraphNodeView: public View, public Component
    {
	public:
	    GraphNodeView(IAppCtx& _ctx): View(_ctx) { m_new_node_desired_position = vec2(-1, -1); };
		~GraphNodeView() override = default;

        void        set_owner(Node *) override;
        bool        draw() override ;
        /** Update view (once per frame)*/
        bool        update() override;
        /** Update view given a certain delta time */
        bool        update(float /* delta_time */);
        /** Update view given a certain delta time and a subsample count */
        bool        update(float /* delta_time */, u8_t /* subsample_count */);
        void        create_child_view_constraints();
        void        destroy_child_view_constraints();
		void        add_contextual_menu_item(
                        const std::string &_category,
                        const std::string &_label,
                        std::function<Node *(void)> _lambda,
                        const func_type *_signature);
        void        frame_all_node_views();
        void        frame_selected_node_views();
        void        translate_all(vec2 /* delta */, const std::vector<NodeView*>&);
    private:
        void        frame_views( std::vector<NodeView*>&);

        [[nodiscard]] GraphNode* get_graph_node() const;
        std::vector<ViewConstraint>                  m_child_view_constraints;
		std::multimap<std::string, FunctionMenuItem> m_contextual_menus;
        vec2                                         m_new_node_desired_position;
        static constexpr const char* k_context_menu_popup = "GraphNodeView.ContextMenu";
        static constexpr const char* k_operator_menu_label = "Operators";
        static constexpr const char* k_function_menu_label = "Functions";

		REFLECT_DERIVED_CLASS()

    };
}