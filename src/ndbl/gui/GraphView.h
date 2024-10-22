#pragma once

#include <map>
#include <string>
#include <functional>

#include "tools/gui/ViewState.h"  // base class
#include "tools/core/reflection/reflection"

#include "ndbl/core/NodeComponent.h"  // base class
#include "ndbl/core/IScope.h"
#include "ndbl/core/Scope.h"

#include "Action.h"

#include "NodeView.h"
#include "SlotView.h"
#include "types.h"
#include "ViewItem.h"
#include "tools/core/StateMachine.h"
#include "CreateNodeCtxMenu.h"

namespace ndbl
{
    // forward declarations
    class Nodable;
    class Graph;
    using tools::Vec2;

    enum SelectionMode
    {
        SelectionMode_ADD     = 0,
        SelectionMode_REPLACE = 1,
    };

    class GraphView
    {
        REFLECT_BASE_CLASS()
    public:
        typedef std::vector<NodeView*> NodeViewVec;
        typedef tools::StateMachine    StateMachine;

	    explicit GraphView(Graph* graph);
		~GraphView() = default;

        void        update(float dt);
        bool        draw();
        void        add_action_to_node_menu(Action_CreateNode* _action);
        void        frame_nodes(FrameMode mode );
        bool        selection_empty() const;
        void        reset(); // unfold and frame the whole graph
        bool        has_an_active_tool() const;
        void        set_selected(const NodeViewVec&, SelectionMode = SelectionMode_REPLACE);
        const NodeViewVec& get_selected() const;
        void        reset_all_properties();
        std::vector<NodeView*> get_all_nodeviews() const;
        static void       draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos);
        Graph*            get_graph() const;
        void              add_child(NodeView*);
        tools::ViewState* view_state() { return &m_view_state; };
        void              _on_add_node(Node* node);
        void              _on_graph_changed();
    private:
        CreateNodeCtxMenu      m_create_node_menu = {};
        ViewItem               m_hovered{};
        ViewItem               m_focused{};
        std::vector<NodeView*> m_selected_nodeview;
        tools::ViewState       m_view_state;
        Graph*                 m_graph;

        void        unfold(); // unfold the graph until it is stabilized
        void        _update(float dt, u16_t samples);
        void        _update(float dt);
        bool        is_selected(NodeView*) const;
        void        frame_views(const std::vector<NodeView*>&, bool _align_top_left_corner);
        void        draw_create_node_context_menu(CreateNodeCtxMenu& menu, SlotView* dragged_slotview = nullptr );

        // Tools State Machine
        //--------------------

        // The data (for some states)

        tools::StateMachine    m_state_machine;
        tools::Vec2            m_roi_state_start_pos;
        tools::Vec2            m_roi_state_end_pos;

        // The behavior

        void cursor_state_tick();
        void roi_state_enter();
        void roi_state_tick();
        void drag_state_enter();
        void drag_state_tick();
        void view_pan_state_tick();
        void line_state_enter();
        void line_state_tick();
        void line_state_leave();

    };
}