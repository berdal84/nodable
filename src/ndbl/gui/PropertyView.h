#pragma once

#include <unordered_map>

#include "tools/core/geometry/Rect.h"
#include "tools/core/memory/memory.h"

#include "ndbl/core/Node.h"
#include "ndbl/core/VariableNode.h"

namespace ndbl
{
    // forward declarations
    class Property;
    class NodeView;
    using tools::ID;
    using tools::PoolID;

    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        ID<Property>     property_id;
        PoolID<NodeView> node_view;
        tools::Rect         screen_rect;
        bool             show_input;
        bool             touched;

        PropertyView();
        PropertyView( PoolID<NodeView> _node_view, ID<Property> _id );
        PropertyView (const PropertyView&);

        void             reset();
        Property*        get_property() const;
        Node*            get_node() const;
        VariableNode*    get_connected_variable() const;
        bool             has_input_connected() const;
    };
}