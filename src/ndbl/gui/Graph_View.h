#pragma once

#include <map>
#include <string>
#include <functional>
#include <vector>

#include "tools/core/Component.h"  // base class
#include "tools/core/Variant.h"
#include "tools/core/Unique_Variant_List.h"
#include "tools/gui/View_State.h"
#include "tools/gui/geometry/Pivots.h"
#include "tools/gui/Size.h"

#include "ndbl/core/Scope.h"

#include "Action.h"
#include "Node_View.h"
#include "Node_Slot_View.h"
#include "tools/core/State_Machine.h"
#include "Node_View_Contextual_Menu.h"
#include "Scope_View.h"

namespace ndbl
{
    // forward declarations
    class  Nodable;
    class  Graph;
    struct ViewConstraint;
    struct Node_Slot_Link_View;
    using  tools::Vec2;

    struct Node_Slot_Link_View
    {
        Node_Slot_View* tail = nullptr;
        Node_Slot_View* head = nullptr;
        bool operator==(const Node_Slot_Link_View& other) const // required to compare tools::Variant<..., Node_Slot_Link_View>
        { return tail == other.tail && head == other.head; }
    };

    using Selectable = tools::VariantT<Node_View*, Scope_View*, Node_Slot_View*, Node_Slot_Link_View> ;
    using Selection  = tools::Unique_Variant_List<Selectable> ;

    class Graph_View : public tools::Component<Graph>
    {
    public:
	    Graph_View();
		~Graph_View() override;

        tools::Simple_Signal signal_change;

        void                   update(float dt);
        bool                   draw(float dt);
        void                   add_action_to_node_menu(Action_CreateNode* _action);
        void                   frame_content(Frame_Mode mode );
        void                   reset(); // unfold and frame the whole graph
        bool                   has_an_active_tool() const;
        Selection&             selection() { return _m_selection; }
        const Selection&       selection() const { return _m_selection; }
        void                   reset_all_properties();
        Graph*                 graph() const { return entity; } // alias for entity
        static void            draw_wire_from_slot_to_pos(Node_Slot_View *from, const Vec2 &end_pos);
    private:
        tools::Spatial_Node*    spatial_node() { return _m_shape.spatial_node(); }
        Node_View_Contextual_Menu      _m_create_node_menu;
        Selectable             _m_hovered;
        Selectable             _m_focused;
        Selection              _m_selection;
        tools::Box_2D          _m_shape;
        bool                   _m_physics_dirty = false;
        std::vector<ViewConstraint> _m_contraints;

        void                   _handle_init();
        void                   _handle_shutdown();
        void                   _handle_add_node(Node* node);
        void                   _handle_remove_node(Node* node);
        void                   _handle_change_scope(Graph::Scope_Change);
        void                   _handle_hover(Scope_View *scope_view);
        void                   _update_until_unfold();
        void                   _update_once(float dt);
        void                   _on_graph_change();
        void                   _on_selection_change(Selection::Event_Type, Selection::Element );
        void                   _draw_create_node_context_menu(Node_View_Contextual_Menu&, Node_Slot_View* dragged_slotview = nullptr );
        void                   _create_constraints__align_top_recursively(const std::vector<Node*>& follower, ndbl::Node *leader);
        void                   _create_constraints__align_down(Node* follower, const std::vector<Node*>& leader);
        void                   _create_constraints(Scope *scope);

        // Tools State Machine
        //--------------------

        // The data (for some states)

        tools::State_Machine    _m_state_machine;
        tools::Vec2            _m_state_roi_start_pos;
        tools::Vec2            _m_state_roi_end_pos;

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

    
    // Set of data and rules to apply constraints to 1 or more views
    // See each rule in rule_xxx(ViewConstraint* constraint, float dt) functions.
    struct ViewConstraint
    {
        // Types

        using Views = std::vector<Node_View*>;
        using Rule  = void(*)(ViewConstraint*, float);

        // Data

        const char*   name           = "untitled Node_ViewConstraint";
        Node_ViewFlags leader_flags   = Node_ViewFlag_WITH_PINNED;
        Node_ViewFlags follower_flags = Node_ViewFlag_WITH_PINNED;
        tools::Vec2   leader_pivot   = tools::RIGHT;
        tools::Vec2   follower_pivot = tools::LEFT;
        tools::Vec2   row_direction  = tools::RIGHT;
        tools::Vec2   gap_direction  = tools::CENTER;
        tools::Size   gap_size       = tools::Size_DEFAULT;
        Views         leader;
        Views         follower;
        Rule          rule;
    };

    // Functions (rules to assign to ViewConstraint.rule)

    static void   ViewConstraintRule_1_to_N_as_row(ViewConstraint*, float dt);
    static void   ViewConstraintRule_N_to_1_as_a_row(ViewConstraint*, float dt);
    static void   ViewConstraintRule_distribute_sub_scope_views(ViewConstraint*, float _dt);
}

// Custom hash provided to work in std::hash<std::Variant<Node_Slot_Link_View, ...>>
template<>
struct std::hash<ndbl::Node_Slot_Link_View>
{
    std::size_t operator()(const ndbl::Node_Slot_Link_View& edge) const noexcept
    { return tools::Hash::hash(edge); }
};
