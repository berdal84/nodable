#pragma once

#include <map>
#include <string>
#include <functional>

#include "fw/gui/View.h"  // base class
#include "fw/core/reflection/reflection"

#include "core/Component.h"  // base class
#include "core/IScope.h"

#include "Action.h"
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

    struct CreateNodeContextMenu
    {
        bool                     must_be_reset_flag   = false;
        ImVec2                   opened_at_pos        = ImVec2(-1,-1); // relative
        ImVec2                   opened_at_screen_pos = ImVec2(-1,-1); // absolute (screen space)
        SlotView*                dragged_slot         = nullptr;  // The slot being dragged when the context menu opened.
        char                     search_input[255]    = "\0";     // The search input entered by the user.
        std::vector<CreateNodeAction*> items;                           // All the available items
        std::vector<CreateNodeAction*> items_with_compatible_signature; // Only the items having a compatible signature (with the slot dragged)
        std::vector<CreateNodeAction*> items_matching_search;           // Only the items having a compatible signature AND matching the search_input.

        CreateNodeAction*        draw_search_input( size_t _result_max_count ); // Return the triggered action, user has to deal with the Action.
        void                     reset_state(SlotView* _dragged_slot = nullptr);
        void                     update_cache_based_on_signature();
        void                     update_cache_based_on_user_input( size_t _limit );
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
        void        add_action_to_context_menu(CreateNodeAction* _action);
    private:
        void        draw_grid( ImDrawList*, const Config& ) const;
        void        frame_views(const std::vector<NodeView *> &_views, bool _align_top_left_corner);
        void        translate_view(ImVec2 vec2);

        Graph*      m_graph;
        ImVec2      m_view_origin;
        CreateNodeContextMenu m_create_node_context_menu;

		REFLECT_DERIVED_CLASS()
    };
}