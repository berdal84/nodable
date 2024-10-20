#pragma once

#include <unordered_map>
#include "tools/gui/ViewState.h"
#include "ViewDetail.h"

namespace ndbl
{
    // forward declarations
    class Property;
    class NodeView;
    class Node;
    class VariableNode;
    class Slot;

    /**
     * Simple struct to store a get_value view state
     */
    class PropertyView
    {
    public:

        bool        show_input;
        bool        touched;

        PropertyView(Property*);

        bool             draw(ViewDetail); // return true when changed
        void             reset();
        Property*        get_property() const;
        Node*            get_node() const;
        Slot*            get_connected_slot() const;
        VariableNode*    get_connected_variable() const;
        bool             has_input_connected() const;

        inline const tools::ViewState*     view_state() const { return &m_view_state; };
        inline tools::ViewState*           view_state() { return &m_view_state; };
        inline const tools::BoxShape2D*    box() const { return &m_view_state.box; };
        inline tools::BoxShape2D*          box() { return &m_view_state.box; };
        inline tools::SpatialNode2D*       xform() { return &m_view_state.box.xform; };
        inline const tools::SpatialNode2D* xform()const  { return &m_view_state.box.xform; };

    private: static float calc_input_width(const char* text);
    public:  static bool  draw_input(PropertyView*, bool _compact_mode, const char* _override_label);
    public:  static bool  draw_all(const std::vector<PropertyView*>&, ViewDetail);
    private:
        Property*        m_property;
        tools::ViewState m_view_state;
    };
}