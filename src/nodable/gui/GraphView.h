#pragma once

#include <map>
#include <string>
#include <functional>

#include "fw/gui/View.h"  // base class
#include "fw/core/reflection/reflection"

#include "core/Component.h"  // base class
#include "core/IScope.h"

#include "types.h"
#include "NodeViewConstraint.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;

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
        bool        update();
        bool        update(float /* delta_time */);
        bool        update(float /* delta_time */, i16_t /* subsample_count */);
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
        void        frame_views(const std::vector<const NodeView *>* _views, bool _align_top_left_corner);

        Graph*      m_graph;
        ImVec2      m_new_node_desired_position;
        std::multimap<std::string, FunctionMenuItem> m_contextual_menus;
        static constexpr const char* k_context_menu_popup = "GraphView.ContextMenu";
        static constexpr const char* k_operator_menu_label = "Operators";
        static constexpr const char* k_function_menu_label = "Functions";

		REFLECT_DERIVED_CLASS()

    };
}