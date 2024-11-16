#pragma once

#include <map>
#include <string>
#include <functional>

#include "tools/gui/ViewState.h"  // base class
#include "tools/core/reflection/reflection"

#include "ndbl/core/NodeComponent.h"  // base class
#include "ndbl/core/Scope.h"

#include "Action.h"

#include "NodeView.h"
#include "SlotView.h"
#include "types.h"
#include "ViewItem.h"
#include "tools/core/StateMachine.h"
#include "CreateNodeCtxMenu.h"
#include "ScopeView.h"
#include "tools/gui/geometry/Pivots.h"

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
    public:
        DECLARE_REFLECT

        typedef std::vector<NodeView*> Selection;
        typedef tools::StateMachine    StateMachine;

	    explicit GraphView(Graph* graph);
		~GraphView();

        SIGNAL(on_change);

        void        update(float dt);
        bool        draw(float dt);
        void        add_action_to_node_menu(Action_CreateNode* _action);
        void        frame_nodes(FrameMode mode );
        bool        selection_empty() const;
        void        reset(); // unfold and frame the whole graph
        bool        has_an_active_tool() const;
        void        set_selected(const Selection&, SelectionMode = SelectionMode_REPLACE);
        const Selection& get_selected() const;
        void        reset_all_properties();
        Graph*            graph() const;
        void              add_child(NodeView*);
        void              decorate_node(Node* node);

        static void       draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos);
    private:
        CreateNodeCtxMenu      m_create_node_menu;
        ViewItem               m_hovered;
        ViewItem               m_focused;
        std::vector<NodeView*> m_selected_nodeview;
        tools::ViewState       m_view_state;
        Graph*                 m_graph;
        bool                   m_physics_dirty = false;

        void        _set_hovered(ScopeView*);
        void        unfold(); // unfold the graph until it is stabilized
        void        _update(float dt, u16_t iterations);
        void        _update(float dt);
        void        _on_graph_change();
        bool        is_selected(NodeView*) const;
        void        frame_views(const std::vector<NodeView*>&, const Vec2& pivot );
        void        draw_create_node_context_menu(CreateNodeCtxMenu& menu, SlotView* dragged_slotview = nullptr );
        void        create_constraints__align_above_recursively(const std::vector<Node*>& unfiltered_follower, ndbl::Node *leader);
        void        create_constraints__align_below(Node* leader, const std::vector<Node*>& follower);
        void        create_constraints_for_scope(Scope *scope);

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