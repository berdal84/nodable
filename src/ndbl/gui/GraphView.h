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
#include "SlotView.h"
#include "types.h"
#include "GraphViewTool.h"
#include "ViewItem.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;
    using tools::Vec2;

    struct GraphViewToolContext
    {
        constexpr static const char* POPUP_NAME = "GraphView.ContextMenuPopup";

        struct ContextMenu
        {
            bool              open_last_frame = false;
            bool              open_this_frame = false;
            tools::Vec2       mouse_pos       = {};
            CreateNodeCtxMenu node_menu       = {};
        };

        ContextMenu context_menu{};
        tools::Vec2 mouse_pos{};
        ImDrawList* draw_list{nullptr};
        ViewItem    hovered{};
        ViewItem    focused{};
        std::vector<NodeView*> selected_nodeview;
        GraphView*  graph_view{nullptr};

        SlotView*   get_focused_slotview() const;
        tools::Vec2 mouse_pos_snapped() const;
        bool        begin_context_menu(); // ImGui style:   if ( begin_..() ) { ...code...  end_..() }
        void        end_context_menu(bool show_search);

        void open_popup() const;
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
        // TODO: move this to state machine ?
        void        set_selected(const NodeViewVec&, SelectionMode = SelectionMode_REPLACE);
        // TODO: move this to state machine ?
        const NodeViewVec& get_selected() const;
        void        reset_all_properties();
        std::vector<NodeView*> get_all_nodeviews() const;
        void        draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos);
        Graph*      get_graph() const;

    private:
        void        unfold(); // unfold the graph until it is stabilized
        bool        update(float dt);
        bool        update(float dt, u16_t samples);
        // // TODO: move this to state machine ?
        bool        is_selected(NodeView*) const;
        void        frame_views(const std::vector<NodeView*>&, bool _align_top_left_corner);

        Graph*      m_graph;
        GraphViewToolContext      m_tool_context;
        GraphViewToolStateMachine m_tool_state_machine;
    };
}