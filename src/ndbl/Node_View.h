#pragma once

#include "bdc/String.hpp"
#include "bdc/Hash_Map.hpp"
#include "reflection/GETTERS_SETTERS.h"
#include "Node_Property.h"
#include "geometry/Box_2D.h"
#include "geometry/Spatial_Node.h"
#include "geometry/Vec4.h"
#include "View_Flags.h"
#include "Config.h"
#include "Node_Property_View.h"
#include "Node_Slot_View.h"

namespace ndbl
{
    // forward declaration
    struct Node;
    struct Node_Slot;
    struct Node_Slot_View;

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

    enum Property_Group
    {
        Property_Group_IN_STRICTLY = 0,
        Property_Group_OUT_STRICTLY,
        Property_Group_INOUT_STRICTLY,
        Property_Group_IN,
        Property_Group_OUT,
        Property_Group_COUNT
    };

    struct Node_View
	{
        Box_2D                                                   shape;
        View_Flags                                               flags  = 0;
        const Vec4*                                              colors[Color_COUNT] = {&Config::COLOR_ERROR};
        bdc::Resizable_Array<Node_Slot_View*>                    slot_views;
        bdc::Hash_Map<const Node_Property*, Node_Property_View*> propertyviews;
        bdc::Resizable_Array<Node_Property_View*>                propertyviews_by_group[Property_Group_COUNT] = {};
        Node_Property_View*                                      value_view              = nullptr;
        Node_Slot_View*                                          hovered_slotview        = nullptr;
        float                                                    opacity                 = 1.f;
        bool                                                     is_expanded             = true;
        Node*                                                    node;

        GETTER(Spatial_Node&, spatial_node, shape.spatial_node);
    };

    void                    nodeview_init(Node_View* view, Node* owner);
    void                    nodeview_deinit(Node_View* view);
    bdc::Array<Node_View*>  nodeview_get_adjacent(const Node_View*, Node_Slot::Flags);
    bool                    nodeview_draw(Node_View*);
    void                    nodeview_update(Node_View*, float);
    void                    nodeview_arrange_recursively(Node_View*, bool smoothly = true);
    bdc::String             nodeview_get_label(const Node_View*);
    inline Rect             nodeview_get_rect(const Node_View* node_view, Space space = WORLD_SPACE) { return box2d_to_rect(node_view->shape, space); }
    Rect                    nodeview_get_rect_ex(const Node_View*, Space, Node_View_Flags);
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
