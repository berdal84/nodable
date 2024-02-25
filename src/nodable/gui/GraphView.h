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
#include "SlotView.h"
#include "core/Scope.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;

	class ContextMenuItem
    {
    public:
        std::string                       label;
        std::string                       search_target_string;
        std::function<PoolID<Node>(void)> create_node_fct;
        const fw::func_type*              function_signature = nullptr;
	};

    struct ContextMenuState {
        bool                                        must_be_reset_flag   = false;
        ImVec2                                      opened_at_pos        = ImVec2(-1,-1); // relative
        ImVec2                                      opened_at_screen_pos = ImVec2(-1,-1); // absolute (screen space)
        SlotView*                                   dragged_slot         = nullptr;  // The slot being dragged when the context menu opened.
        char                                        search_input[255]    = "\0";     // The search input entered by the user.
        std::multimap<std::string, ContextMenuItem> items_by_category;               // All the available items sorted by category
        std::vector<ContextMenuItem>                items_with_compatible_signature; // Only the items having a compatible signature (with the slot dragged)
        std::vector<ContextMenuItem>                items_matching_search;           // Only the items having a compatible signature AND matching the search_input.
        void reset_state(SlotView* _dragged_slot = nullptr);
        void add_item(
                const std::string&                _category_key,         // The category key to put the item into.
                const std::string&                _label,                // The displayed label for this item.
                std::string                       _search_target_string, // The string used to search (will be converted to lower case).
                std::function<PoolID<Node>(void)> _node_factory_fct,     // The lambda function to invoke to create the node linked to this menu item.
                const fw::func_type *_signature = nullptr);
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
        void        frame_all_node_views();
        void        frame_selected_node_views();
        void        translate_all(ImVec2 /* delta */, const std::vector<NodeView*>&);
        void        unfold(); // unfold the graph until it is stabilized

    private:
        void                 draw_grid( ImDrawList*, const Config& ) const;
        bool                 draw_search_input( size_t _result_max_count ); // Search input field, return true if user created a node.
        void                 frame_views(const std::vector<NodeView *> &_views, bool _align_top_left_corner);
        void                 translate_view(ImVec2 vec2);
        void                 close_popup_context_menu();
        PoolID<VariableNode> create_variable(const fw::type* _type, const char*  _name, PoolID<Scope>  _scope);

        Graph*               m_graph;
        ImVec2               m_view_origin;
        ContextMenuState     m_context_menu;
        PoolID<Node>         m_new_node_id; // contains the last id created, cleared each frame
        static constexpr const char* k_context_menu_popup = "GraphView.ContextMenu";
        static constexpr const char* k_operator_menu_label = "Operators";
        static constexpr const char* k_function_menu_label = "Functions";

		REFLECT_DERIVED_CLASS()
    };
}