#pragma once

#include <functional>
#include <vector>

#include "core/reflection/GETTERS_SETTERS.h"
#include "tools/core/Component.h"  // base class
#include "tools/core/Variant.h"
#include "tools/core/Unique_Variant_List.h"
#include "tools/gui/geometry/Pivots.h"
#include "tools/gui/Size.h"

#include "ndbl/core/Scope.h"

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
    struct Node_View_Constraint;
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

    struct Graph_View : public tools::Component<Graph>
    {
	    Graph_View();
		~Graph_View();

        tools::Simple_Signal                signal_change;
        Node_View_Contextual_Menu           contextual_menu;
        Selectable                          hovered;
        Selectable                          focused;
        Selection                           selection;
        tools::Box_2D                       shape;
        std::vector<Node_View_Constraint>   contraints;
        bool                                is_physics_dirty = false;

        tools::State_Machine                state_machine;
        tools::Vec2                         state_roi_start_pos;
        tools::Vec2                         state_roi_end_pos;   
        
        GETTER(Graph*, graph, entity);
    };

    void    graphview_update(Graph_View*, float dt);
    bool    graphview_draw(Graph_View*, float dt);
    void    graphview_frame_content(Graph_View*, Frame_Mode);
    void    graphview_reset(Graph_View*); // unfold and frame the whole graph
    bool    graphview_has_an_active_tool(const Graph_View*);
    void    graphview_reset_all_properties(Graph_View*);
    
    // Set of data and rules to apply constraints to 1 or more views
    // See each rule in rule_xxx(ViewConstraint* constraint, float dt) functions.
    struct Node_View_Constraint
    {
        using Rule  = void(*)(Node_View_Constraint*, float);

        // Data

        std::vector<Node_View*> leader          = {};
        std::vector<Node_View*> follower        = {};
        const char*             name            = "untitled Node_View_Constraint";
        Node_View_Flags         leader_flags    = Node_View_Flag_WITH_PINNED;
        Node_View_Flags         follower_flags  = Node_View_Flag_WITH_PINNED;
        tools::Vec2             leader_pivot    = tools::RIGHT;
        tools::Vec2             follower_pivot  = tools::LEFT;
        tools::Vec2             row_direction   = tools::RIGHT;
        tools::Vec2             gap_direction   = tools::CENTER;
        tools::Size             gap_size        = tools::Size_DEFAULT;
        Rule                    rule            = nullptr;
    };

    // rules to assign to Node_View_Constraint.rule

    void nodeviewcontraint_rule_1_to_N_as_row               (Node_View_Constraint*, float dt);
    void nodeviewcontraint_rule_N_to_1_as_a_row             (Node_View_Constraint*, float dt);
    void nodeviewcontraint_rule_distribute_sub_scope_views  (Node_View_Constraint*, float dt);
}

// Custom hash provided to work in std::hash<std::Variant<Node_Slot_Link_View, ...>>
template<>
struct std::hash<ndbl::Node_Slot_Link_View>
{
    std::size_t operator()(const ndbl::Node_Slot_Link_View& edge) const noexcept
    { return tools::Hash::hash(edge); }
};
