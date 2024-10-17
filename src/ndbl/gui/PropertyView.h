#pragma once

#include <unordered_map>
#include "tools/gui/ViewState.h"

namespace ndbl
{
    // forward declarations
    class Property;
    class NodeView;
    class Node;
    class VariableNode;
    class Slot;

    /**
     * Simple struct to store a get_value view state_handle
     */
    class PropertyView
    {
    public:
        bool        show_input;
        bool        touched;

        PropertyView(Property*);

        void             reset();
        Property*        get_property() const;
        Node*            get_node() const;
        Slot*            get_connected_slot() const;
        VariableNode*    get_connected_variable() const;
        bool             has_input_connected() const;
        tools::ViewState* get_state() { return &m_state; }
        tools::Box*      box() { return &m_state.box; };
        tools::XForm2D*  xform() { return &m_state.box.xform; };

    private:
        Property*        m_property;
        tools::ViewState m_state;
    };
}