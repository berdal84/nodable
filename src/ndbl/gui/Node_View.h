#pragma once

#include <unordered_map>

#include "bdc/String.hpp"
#include "ndbl/core/reflection/GETTERS_SETTERS.h"
#include "ndbl/core/Node_Property.h"
#include "geometry/Spatial_Node.h"
#include "geometry/Box_2D.h"
#include "View_Flags.h"
#include "Config.h"
#include "Node_Property_View.h"
#include "Node_Slot_View.h"

namespace ndbl
{
    // forward declaration
    struct Node;
    struct Graph;
    struct Scope_View;
    struct Node_Slot;
    struct Node_Slot_View;
    struct Graph_View;

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

    struct Node_View
	{
        Box_2D                               shape;
        View_Flags                           flags  = 0;
        std::array<const Vec4*, Color_COUNT> colors = {&Config::COLOR_ERROR};
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
        bool                                        is_expanded             = true;
        Node*                                       node;

        GETTER(Spatial_Node&, spatial_node, shape.spatial_node);
    };

    void                    nodeview_init(Node_View* view, Node* owner);
    void                    nodeview_deinit(Node_View* view);
    std::vector<Node_View*> nodeview_get_adjacent(const Node_View*, Node_Slot::Flags);
    bool                    nodeview_draw(Node_View*);
    void                    nodeview_update(Node_View*, float);
    void                    nodeview_arrange_recursively(Node_View*, bool smoothly = true);
    bdc::String             nodeview_get_label(const Node_View*);
    inline Rect      nodeview_get_rect(const Node_View* node_view, Space space = WORLD_SPACE) { return node_view->shape.rect(space); }
    Rect             nodeview_get_rect_ex(const Node_View*, Space, Node_View_Flags);
    void                    nodeview_set_visible_recursively(Node_View*, bool);
    void                    nodeview_toggle_expandcollapse(Node_View*);
    void                    nodeview_reset_all_properties(Node_View*);
    bool                    nodeview_draw_as_properties_panel(Node_View*, bool* show_advanced );
    Node_View*              nodeview_substitute_with_parent_if_not_visible(Node_View*, bool _recursive = true);
    void                    nodeview_handle_init(Node_View*);
    void                    nodeview_handle_deinit(Node_View*);
    Node_Property_View*     nodeview_find_property_view(Node_View*, const Node_Property*);
    void                    nodeview_draw_slot(Node_View*, Node_Slot_View*);
    void                    nodeview_draw_node_rect(Rect rect,
                                                    Vec4 color, Vec4 border_highlight_col, Vec4 shadow_col, Vec4 border_col,
                                                    float border_radius, float border_width);
}
