#pragma once

#include <map>
#include <string>
#include <functional>

#include "fw/gui/View.h"  // base class
#include "fw/core/reflection/reflection"

#include "core/IScope.h"
#include "core/Component.h"  // base class

#include "types.h"     // forward declarations
#include "NodeView.h" // for NodeViewConstraint

namespace ndbl
{
    // forward declarations
    class Nodable;

	typedef struct {
        std::string                 label;
        std::function<Node *(void)> create_node_fct;
        const fw::func_type*        function_signature;
	} FunctionMenuItem;

    class GraphView: public fw::View
    {
	public:
	    GraphView(Graph* graph);
		~GraphView() override = default;
        /** Update view (once per frame)*/
        bool        update();
        /** Update view given a certain delta time */
        bool        update(float /* delta_time */);
        /** Update view given a certain delta time and a subsample count */
        bool        update(float /* delta_time */, i16_t /* subsample_count */);
        void        create_child_view_constraints();
        void        destroy_child_view_constraints();
		void        add_contextual_menu_item(
                        const std::string &_category,
                        const std::string &_label,
                        std::function<Node *(void)> _lambda,
                        const fw::func_type *_signature);
        void        frame_all_node_views();
        void        frame_selected_node_views();
        void        translate_all(ImVec2 /* delta */, const std::vector<NodeView*>&);
        void        unfold(); // unfold the graph until it is stabilized

    private:
        bool        draw_implem() override ;
        void        frame_views( std::vector<NodeView*>&);
        Graph*      get_graph_node() const;

        Graph*      m_graph;
        std::vector<ViewConstraint>                  m_child_view_constraints;
		std::multimap<std::string, FunctionMenuItem> m_contextual_menus;
        ImVec2                                       m_new_node_desired_position;
        static constexpr const char* k_context_menu_popup = "GraphView.ContextMenu";
        static constexpr const char* k_operator_menu_label = "Operators";
        static constexpr const char* k_function_menu_label = "Functions";

		REFLECT_DERIVED_CLASS()

    };
}