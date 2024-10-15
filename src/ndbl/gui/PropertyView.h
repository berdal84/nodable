#pragma once

#include <unordered_map>
#include "tools/gui/View.h"

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
    class PropertyView : public tools::View
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
    private:
        Property*        m_property;
    };
}