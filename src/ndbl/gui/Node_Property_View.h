#pragma once

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

        bool        show;
        bool        touched;

        Node_Property_View(Node_Property*);

        bool            draw(View_Detail); // return true when changed
        void            reset();
        Node_Property*  get_property() const;
        Node*           get_node() const;
        Node_Slot*      get_connected_slot() const;
        Node*           get_connected_variable() const;
        bool            has_input_connected() const;

        const tools::View_State*    state() const { return &_state; };
        tools::View_State*          state() { return &_state; };
        const tools::Box_2D*        shape() const { return &_shape; };
        tools::Box_2D*              shape() { return &_shape; };
        tools::Spatial_Node*        spatial_node() { return _shape.spatial_node(); };
        const tools::Spatial_Node*  spatial_node()const  { return _shape.spatial_node(); };

    private: static float calc_input_width(const char* text);
    public:  static bool  draw_input(Node_Property_View*, bool _compact_mode, const char* _override_label);
    public:  static bool  draw_all(const std::vector<Node_Property_View*>&, View_Detail);
    private:
        Node_Property*      _property;
        tools::View_State   _state;
        tools::Box_2D       _shape;
    };
}