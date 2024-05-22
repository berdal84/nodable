#pragma once

#include <map>
#include <string>
#include <functional>

#include "fw/gui/View.h"  // base class
#include "fw/core/reflection/reflection"

#include "nodable/core/Component.h"  // base class
#include "nodable/core/IScope.h"
#include "nodable/core/Scope.h"

#include "Action.h"

#include "NodeView.h"
#include "NodeViewConstraint.h"
#include "SlotView.h"
#include "types.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;

    struct CreateNodeContextMenu
    {
        bool                     must_be_reset_flag   = false;
        fw::Vec2 opened_at_pos        = fw::Vec2(-1,-1); // relative
        fw::Vec2 opened_at_screen_pos = fw::Vec2(-1,-1); // absolute (screen space)
        SlotView*                dragged_slot         = nullptr;  // The slot being dragged when the context menu opened.
        char                     search_input[255]    = "\0";     // The search input entered by the user.
        std::vector<Action_CreateNode*> items;                           // All the available items
        std::vector<Action_CreateNode*> items_with_compatible_signature; // Only the items having a compatible signature (with the slot dragged)
        std::vector<Action_CreateNode*> items_matching_search;           // Only the items having a compatible signature AND matching the search_input.

        Action_CreateNode*        draw_search_input( size_t _result_max_count ); // Return the triggered action, user has to deal with the Action.
        void                     reset_state(SlotView* _dragged_slot = nullptr);
        void                     update_cache_based_on_signature();
        void                     update_cache_based_on_user_input( size_t _limit );
    };

    class GraphView: public fw::View
    {
	public:
	    GraphView(Graph* graph);
		~GraphView() override = default;

        bool        onDraw() override;
        bool        update();
        bool        update(float /* delta_time */);
        bool        update(float /* delta_time */, i16_t /* subsample_count */);
        void        frame_all_node_views();
        void        frame_selected_node_views();
        void        translate_all(const std::vector<NodeView*>&, fw::Vec2 delta, NodeViewFlags);
        void        unfold(); // unfold the graph until it is stabilized
        void        add_action_to_context_menu( Action_CreateNode* _action);
        void        frame( FrameMode mode );

    private:
        void        draw_grid( ImDrawList* ) const;
        void        frame_views(const std::vector<NodeView *> &_views, bool _align_top_left_corner);
        void        pan(fw::Vec2); // translate content

        Graph*      m_graph;
        CreateNodeContextMenu m_create_node_context_menu;

		REFLECT_DERIVED_CLASS()
    };
}