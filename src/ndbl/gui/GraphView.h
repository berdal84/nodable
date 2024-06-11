#pragma once

#include <map>
#include <string>
#include <functional>

#include "tools/gui/View.h"  // base class
#include "tools/core/reflection/reflection"

#include "ndbl/core/Component.h"  // base class
#include "ndbl/core/IScope.h"
#include "ndbl/core/Scope.h"

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
    using tools::Vec2;

    struct CreateNodeContextMenu
    {
        bool      must_be_reset_flag   = false;
        Vec2      opened_at_pos        {-1,-1}; // relative
        Vec2      opened_at_screen_pos {-1,-1}; // absolute (screen space)
        SlotView* dragged_slot         = nullptr;  // The slot being dragged when the context menu opened.
        char      search_input[255]    = "\0";     // The search input entered by the user.
        std::vector<Action_CreateNode*> items;                           // All the available items
        std::vector<Action_CreateNode*> items_with_compatible_signature; // Only the items having a compatible signature (with the slot dragged)
        std::vector<Action_CreateNode*> items_matching_search;           // Only the items having a compatible signature AND matching the search_input.

        Action_CreateNode*        draw_search_input( size_t _result_max_count ); // Return the triggered action, user has to deal with the Action.
        void                     reset_state(SlotView* _dragged_slot = nullptr);
        void                     update_cache_based_on_signature();
        void                     update_cache_based_on_user_input( size_t _limit );
    };

    typedef int SelectionMode;
    enum SelectionMode_
    {
        SelectionMode_ADD     = 0,
        SelectionMode_REPLACE = 1,
    };

    struct FrameState
    {
        bool        is_any_wire_dragged{false};
        bool        is_any_wire_hovered{false};
        const Slot* hovered_wire_start{nullptr};
        const Slot* hovered_wire_end{nullptr};
        bool        is_any_node_dragged{false};
        bool        is_any_node_hovered{false};
        bool slotview_dropped_on_background{false};
        NodeView*   hovered_node{ nullptr};
    };

    typedef int SlotFlags;
    enum SlotFlags_
    {
        SlotBehavior_READONLY       = 1 << 0,
        SlotBehavior_ALLOW_DRAGGING = 1 << 1,
    };

    class GraphView: public tools::View
    {
	public:
        using NodeViewVec = std::vector<PoolID<NodeView>>;

	    GraphView(Graph* graph);
		~GraphView() override = default;

        bool        onDraw() override;
        bool        update();
        bool        update(float /* delta_time */);
        bool        update(float /* delta_time */, i16_t /* subsample_count */);
        void        frame_all_node_views();
        void        frame_selected_node_views();
        void        translate_all(const std::vector<NodeView*>&, Vec2 delta, NodeViewFlags);
        void        unfold(); // unfold the graph until it is stabilized
        void        add_action_to_context_menu( Action_CreateNode* _action);
        void        frame( FrameMode mode );
        void        set_selected(PoolID<NodeView> new_selection, SelectionMode mode);
        bool        is_any_dragged() const;
        bool        is_selected(PoolID<NodeView>) const;
        bool        is_dragged(PoolID<NodeView> ) const;
        const NodeViewVec& get_selected() const;
        const NodeViewVec& get_dragged() const;

        void set_hovered_slotview(SlotView *pView);

        SlotView *get_dragged_slotview() const;

        void set_dragged_slotview(SlotView *pView);

        SlotView *get_hovered_slotview() const;

    private:
        void        draw_grid( ImDrawList* ) const;
        void        frame_views(const std::vector<NodeView *> &_views, bool _align_top_left_corner);
        void        pan(Vec2); // translate content

        Graph*      m_graph;
        CreateNodeContextMenu m_create_node_menu{};
        FrameState m_last_frame;
        NodeViewVec m_selected_nodeview;
        NodeViewVec m_dragged_nodeview;
        SlotView*   m_hovered_slotview;
        SlotView*   m_focused_slotview;
        SlotView*   m_dragged_slotview;
		REFLECT_DERIVED_CLASS()
    };
}