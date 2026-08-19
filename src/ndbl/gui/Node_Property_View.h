#pragma once

#include "gui/geometry/Box_2D.h"
#include "tools/gui/View_Flags.h"
#include "Config.h"

namespace ndbl
{
    // forward declarations
    class Node_Property;
    class Node_View;
    class Node;
    struct Node_Slot;

    /**
     * Simple struct to store a get_value view state
     */
    struct Node_Property_View
    {
        bool                show;       // TODO: move this to State_View?
        bool                touched;    // TODO: move this to State_View?
        Node_Property*      property;
        tools::View_Flags   flags;
        tools::Box_2D       shape;

        Node*               node() const;
        Node_Slot*          connected_slot() const;
        Node*               connected_variable() const;
        bool                has_input_connected() const;
    };

    void  nodepropertyview_init(Node_Property_View*, Node_Property* property);
    bool  nodepropertyview_draw(Node_Property_View*, View_Detail); // return true when changed
    void  nodepropertyview_reset(Node_Property_View*);
    bool  nodepropertyview_draw_input(Node_Property_View*, bool _compact_mode, const bdc::String& _override_label);
    bool  nodepropertyview_draw_all(const std::vector<Node_Property_View*>&, View_Detail);
    float nodepropertyview_calc_input_width(const bdc::String& text);
}