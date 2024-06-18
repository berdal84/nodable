#pragma once

#include <map>
#include <string>
#include <functional>

#include "tools/gui/View.h"  // base class
#include "tools/core/reflection/reflection"

#include "ndbl/core/NodeComponent.h"  // base class
#include "ndbl/core/IScope.h"
#include "ndbl/core/Scope.h"

#include "Action.h"

#include "NodeView.h"
#include "NodeViewConstraint.h"
#include "SlotView.h"
#include "types.h"
#include "GraphViewTool.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;
    using tools::Vec2;

    class CreateNodeContextMenu
    {
    public:
        Action_CreateNode*       draw_search_input(SlotView* _dragged_slot, size_t _result_max_count ); // Return the triggered action, user has to deal with the Action.
        void                     reset_state();
        void                     add_action(Action_CreateNode*);

    private:
        bool      must_be_reset_flag   = false;
        char      search_input[255]    = "\0";     // The search input entered by the user.
        std::vector<Action_CreateNode*> items;                           // All the available items
        std::vector<Action_CreateNode*> items_with_compatible_signature; // Only the items having a compatible signature (with the slot dragged)
        std::vector<Action_CreateNode*> items_matching_search;           // Only the items having a compatible signature AND matching the search_input.
        void                     update_cache_based_on_signature(SlotView* _dragged_slot);
        void                     update_cache_based_on_user_input(SlotView* _dragged_slot, size_t _limit );
    };

    typedef int SelectionMode;
    enum SelectionMode_
    {
        SelectionMode_ADD     = 0,
        SelectionMode_REPLACE = 1,
    };

    class GraphView: public tools::View
    {
        REFLECT_DERIVED_CLASS()
	public:
        using NodeViewVec = std::vector<NodeView*>;

	    explicit GraphView(Graph* graph);
		~GraphView() override = default;

        bool        draw() override;
        void        add_action_to_context_menu( Action_CreateNode* _action);
        void        frame_nodes(FrameMode mode );
        bool        selection_empty() const;
        void        reset(); // unfold and frame the whole graph
        bool        update();
        bool        has_an_active_tool() const;
        void        set_selected(const NodeViewVec&, SelectionMode = SelectionMode_REPLACE);
        const NodeViewVec& get_selected() const;
        void        reset_all_properties();

    private:
        void        reset_tool();
        void        change_tool(Tool new_tool);
        void        unfold(); // unfold the graph until it is stabilized
        bool        update(float dt);
        bool        update(float dt, u16_t samples);
        static void translate_all(const std::vector<NodeView*>&, const Vec2& offset, NodeViewFlags);
        void        translate_all(const Vec2& offset);
        bool        is_selected(NodeView*) const;
        void        frame_views(const std::vector<NodeView*>&, bool _align_top_left_corner);
        std::vector<NodeView*> get_all_nodeviews() const;

        Graph*      m_graph;
        CreateNodeContextMenu m_create_node_menu{};
        NodeViewVec m_selected_nodeview;
        Item        m_focused{};
        Tool        m_tool{};
        bool        m_context_menu_open_last_frame{false};
    };
}