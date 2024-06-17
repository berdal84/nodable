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

    /**
     * Simple struct to store a property view state
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
        VariableNode*    get_connected_variable() const;
        bool             has_input_connected() const;
    private:
        Property*        m_property;
    };
}