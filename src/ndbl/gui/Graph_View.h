#pragma once

#include <functional>

#include "core/Types.h"
#include "tools/core/reflection/GETTERS_SETTERS.h"
#include "tools/core/Component.h"  // base class
#include "tools/core/Variant.h"
#include "tools/core/Unique_Variant_List.h"
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

    typedef u8_t Graph_View_Flags;
    enum Graph_View_Flag: u8_t
    {
        Graph_View_Flag_NONE                    = 0,
        Graph_View_Flag_NEEDS_TO_BE_RESET       = 1 << 0,
        Graph_View_Flag_NEEDS_TO_FRAME_CONTENT  = 1 << 1
    };

    struct Graph_View : public tools::Component<Graph>
    {
	    Graph_View();
		~Graph_View();

        Graph_View_Flags                    flags = 0;
        tools::Simple_Signal                signal_change;
        Node_View_Contextual_Menu           contextual_menu;
        Selectable                          hovered;
        Selectable                          focused;
        Selection                           selection;
        tools::Box_2D                       shape;
        tools::State_Machine                state_machine;
        tools::Vec2                         state_roi_start_pos;
        tools::Vec2                         state_roi_end_pos;   
        
        GETTER(Graph*, graph, entity);
    };

    void    graphview_update(Graph_View*, float dt);
    bool    graphview_draw(Graph_View*, float dt);
    bool    graphview_has_an_active_tool(const Graph_View*);
    void    graphview_reset_all_properties(Graph_View*);
}

// Custom hash provided to work in std::hash<std::Variant<Node_Slot_Link_View, ...>>
template<>
struct std::hash<ndbl::Node_Slot_Link_View>
{
    std::size_t operator()(const ndbl::Node_Slot_Link_View& edge) const noexcept
    { return tools::Hash::hash(edge); }
};
