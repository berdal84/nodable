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

        bool        draw();
        void        add_action_to_context_menu( Action_CreateNode* _action);
        void        frame_nodes(FrameMode mode );
        bool        selection_empty() const;
        void        reset(); // unfold and frame the whole graph
        bool        update();
        bool        has_an_active_tool() const;
        void        set_selected(const NodeViewVec&, SelectionMode = SelectionMode_REPLACE);
        const NodeViewVec& get_selected() const;
        void        reset_all_properties();
        std::vector<NodeView*> get_all_nodeviews() const;
        static void        draw_wire_from_slot_to_pos(SlotView *from, const Vec2 &end_pos);
        Graph*      get_graph() const;
        tools::View* base() { return &base_view; };
    private:
        void        unfold(); // unfold the graph until it is stabilized
        bool        update(float dt);
        bool        update(float dt, u16_t samples);
        bool        is_selected(NodeView*) const;
        void        frame_views(const std::vector<NodeView*>&, bool _align_top_left_corner);
        SlotView*   get_focused_slotview() const;
        tools::Vec2 mouse_pos_snapped() const;
        bool        begin_context_menu(); // ImGui style:   if ( begin_..() ) { ...code...  end_..() }
        void        end_context_menu(bool show_search);
        void        open_popup() const;

        tools::View base_view;
        Graph*      m_graph;

        // Tool State Machine & States
        //============================

        constexpr static const char* POPUP_NAME = "GraphView.ContextMenuPopup";

        struct ContextMenu
        {
            bool              open_last_frame = false;
            bool              open_this_frame = false;
            tools::Vec2       mouse_pos       = {};
            CreateNodeCtxMenu node_menu       = {};
        };

        ContextMenu            m_context_menu{};
        ViewItem               m_hovered{};
        ViewItem               m_focused{};
        std::vector<NodeView*> m_selected_nodeview;

        // Tools State Machine
        //--------------------

        // The data (for some states)

        tools::StateMachine    m_state_machine;
        tools::Vec2            m_roi_state_start_pos;
        tools::Vec2            m_roi_state_end_pos;
        SlotView*              m_line_state_dragged_slotview{};

        // The behavior

        void cursor_state_tick();
        void roi_state_enter();
        void roi_state_tick();
        void drag_state_enter();
        void drag_state_tick();
        void view_pan_state_tick();
        void line_state_tick();
    };
}