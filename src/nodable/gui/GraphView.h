#pragma once

#include <map>
#include <string>
#include <functional>

#include "fw/gui/View.h"  // base class
#include "fw/core/reflection/reflection"

#include "core/Component.h"  // base class
#include "core/IScope.h"

#include "Config.h"
#include "NodeViewConstraint.h"
#include "core/Scope.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;

	class FunctionMenuItem {
    public:
        std::string                       label;
        std::function<PoolID<Node>(void)> create_node_fct;
        const fw::func_type*              function_signature = nullptr;
	};

    class GraphView: public fw::View
    {
	public:
	    GraphView(Graph* graph);
		~GraphView() override = default;

        bool        draw() override;
        bool        update();
        bool        update(float /* delta_time */);
        bool        update(float /* delta_time */, i16_t /* subsample_count */);
		void        add_contextual_menu_item(
                        const std::string &_category,
                        const std::string &_label,
                        std::function<PoolID<Node>(void)> _function,
                        const fw::func_type *_signature);
        void        frame_all_node_views();
        void        frame_selected_node_views();
        void        translate_all(ImVec2 /* delta */, const std::vector<NodeView*>&);
        void        unfold(); // unfold the graph until it is stabilized

    private:
        PoolID<Node>         draw_menu(const std::string& _menu_key);
        void                 draw_grid( ImDrawList*, const Config& ) const;
        PoolID<Node>         draw_search_input( size_t _result_max_count ); // Search input filtering nodes as short clickable list
        void                 frame_views(const std::vector<NodeView *> &_views, bool _align_top_left_corner);
        void                 translate_view(ImVec2 vec2);
        PoolID<VariableNode> create_variable(const fw::type* _type, const char*  _name, PoolID<Scope>  _scope);

        Graph*      m_graph;
        ImVec2      m_view_origin;
        bool        m_focus_search_input;
        ImVec2      m_new_node_desired_position;
        char        m_search_input[255] = "\0";
        PoolID<Node> m_new_node_id; // contains the last id created, cleared each frame
        std::multimap<std::string, FunctionMenuItem> m_contextual_menus;
        std::vector<FunctionMenuItem> m_filtered_contextual_menus;
        static constexpr const char* k_context_menu_popup = "GraphView.ContextMenu";
        static constexpr const char* k_operator_menu_label = "Operators";
        static constexpr const char* k_function_menu_label = "Functions";

		REFLECT_DERIVED_CLASS()
        void open_popup_context_menu();
    };
}