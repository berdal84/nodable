#pragma once

#include <string>
#include <unordered_map>

#include "gui/Config.h"
#include "gui/geometry/Spatial_Node.h"
#include "tools/core/Component.h"// base class
#include "tools/core/reflection/GETTERS_SETTERS.h"
#include "tools/gui/geometry/Box_2D.h"
#include "tools/gui/View_State.h"
#include "ndbl/core/Node_Property.h"
#include "Node_Property_View.h"
#include "Node_Slot_View.h"

namespace ndbl
{
    // forward declaration
    class Node;
    class Graph;
    class Scope_View;
    struct Node_Slot;
    struct Node_Slot_View;
    class Graph_View;

    /**
     * Enum to define some color types
     */
    enum ColorType
    {
        Color_FILL,
        Color_COUNT
    };

    typedef int Node_View_Flags;
    enum Node_View_Flag_
    {
        // note: when adding a new value, remember we want NONE to be the most common case

        Node_View_Flag_NONE                   = 0,
        Node_View_Flag_WITH_RECURSION         = 1 << 0,
        Node_View_Flag_WITH_PINNED            = 1 << 1,
        Node_View_Flag_EXCLUDE_UNSELECTED     = 1 << 2
    };

    enum Property_Category
    {
        Property_Category_IN_STRICTLY = 0,
        Property_Category_OUT_STRICTLY,
        Property_Category_INOUT_STRICTLY,
        Property_Category_IN,
        Property_Category_OUT,
        Property_Category_COUNT
    };

    struct Node_View : public tools::Component<Node>
	{
        tools::Box_2D                               shape;
        tools::View_State                           state;
        std::array<const tools::Vec4*, Color_COUNT> colors = {&tools::Config::COLOR_ERROR};
        std::vector<Node_Slot_View*>                slot_views;
        std::unordered_map<
            const Node_Property*,
            Node_Property_View*
        >                                           view_by_property;
        std::array<
        std::vector<Node_Property_View*>,
        Property_Category_COUNT
        >                                           view_by_property_type   = {};
        Node_Property_View*                         value_view              = nullptr;
        Node_Slot_View*                             hovered_slotview        = nullptr;
        Scope_View*                                 internal_scopeview      = nullptr;
        float                                       opacity                 = 1.f;
        bool                                        is_expanded                = true;

        Node_View();
		~Node_View() override;
        
        GETTER(Node*               , node        , entity);
        GETTER(tools::Spatial_Node&, spatial_node, shape.spatial_node);
    };

    std::vector<Node_View*> nodeview_get_adjacent(const Node_View*, Node_Slot::Flags);
    bool                    nodeview_draw(Node_View*);
    void                    nodeview_update(Node_View*, float);
    void                    nodeview_arrange_recursively(Node_View*, bool smoothly = true);
    std::string             nodeview_get_label(const Node_View*);
    inline tools::Rect      nodeview_get_rect(const Node_View* node_view, tools::Space space = tools::WORLD_SPACE) { return node_view->shape.rect(space); }
    tools::Rect             nodeview_get_rect_ex(const Node_View*, tools::Space, Node_View_Flags);
    void                    nodeview_set_expanded_rec(Node_View*, bool);
    void                    nodeview_set_expanded(Node_View*, bool);
    void                    nodeview_set_inputs_visible( Node_View*, bool visible, bool recursive = false);
    void                    nodeview_set_children_visible( Node_View*, bool visible, bool recursive = false);
    inline void             nodeview_expand_toggle( Node_View* node_view ) { nodeview_set_expanded(node_view, !node_view->is_expanded); }
    inline void             nodeview_expand_toggle_rec( Node_View* node_view ) { return nodeview_set_expanded_rec(node_view, !node_view->is_expanded); };
    void                    nodeview_reset_all_properties(Node_View*);
    tools::Rect             nodeview_bounding_rect(const std::vector<Node_View *>&, tools::Space = tools::WORLD_SPACE, Node_View_Flags = Node_View_Flag_NONE);
    bool                    nodeview_draw_as_properties_panel(Node_View*, bool* show_advanced );
    Node_View*              nodeview_substitute_with_parent_if_not_visible(Node_View*, bool _recursive = true);
    void                    nodeview_handle_init(Node_View*);
    void                    nodeview_handle_deinit(Node_View*);
    Node_Property_View*     nodeview_find_property_view(Node_View*, const Node_Property*);
    void                    nodeview_draw_slot(Node_View*, Node_Slot_View*);
    void                    nodeview_set_adjacent_visible(Node_View*, Node_Slot::Flags, bool _visible, Node_View_Flags = Node_View_Flag_NONE);
    void                    nodeview_draw_node_rect(tools::Rect rect,
                                                    tools::Vec4 color, tools::Vec4 border_highlight_col, tools::Vec4 shadow_col, tools::Vec4 border_col,
                                                    bool selected,
                                                    float border_radius, float border_width);
}
