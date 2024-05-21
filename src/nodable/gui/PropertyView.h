#pragma once

#include <unordered_map>

#include "fw/core/geometry/Rect.h"
#include "fw/core/memory/Pool.h"

#include "nodable/core/Node.h"
#include "nodable/core/VariableNode.h"

namespace ndbl
{
    // forward declarations
    class Property;
    class NodeView;
    using fw::ID;
    using fw::PoolID;

    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        ID<Property>     property_id;
        PoolID<NodeView> node_view;
        fw::Rect         screen_rect;
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