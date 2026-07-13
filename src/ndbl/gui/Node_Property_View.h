#pragma once

#include "gui/geometry/Box_2D.h"
#include "tools/gui/View_State.h"
#include "View_Detail.h"

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
    class Node_Property_View
    {
    public:

        bool                show;       // TODO: move this to State_View?
        bool                touched;    // TODO: move this to State_View?
        Node_Property*      property;
        tools::View_State   state;
        tools::Box_2D       shape;

        Node_Property_View(Node_Property*);

        bool                        draw(View_Detail); // return true when changed
        void                        reset();
        Node*                       node() const;
        Node_Slot*                  connected_slot() const;
        Node*                       connected_variable() const;
        bool                        has_input_connected() const;
        tools::Spatial_Node*        spatial_node()          { return shape.spatial_node(); };
        const tools::Spatial_Node*  spatial_node() const    { return shape.spatial_node(); };

        static bool  draw_input(Node_Property_View*, bool _compact_mode, const char* _override_label);
        static bool  draw_all(const std::vector<Node_Property_View*>&, View_Detail);

    private:
        static float calc_input_width(const char* text);
    };
}