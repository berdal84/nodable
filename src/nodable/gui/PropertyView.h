#pragma once

#include "core/Node.h"
#include "core/VariableNode.h"
#include "fw/core/Pool.h"
#include "imgui.h"

namespace ndbl {

    // forward declarations
    class Property;
    class SlotView;
    class NodeView;
    using fw::ID;
    using fw::PoolID;

    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        ID<Property>     property;
        PoolID<NodeView> node_view;
        float            hpos; // horizontal position
        bool             show_input;
        bool             touched;
        std::vector<SlotView> slot_views;

        PropertyView() = default;
        PropertyView( PoolID<NodeView> _node_view, ID<Property> _id );
        PropertyView (const PropertyView&) = default;
        PropertyView& operator= (const PropertyView&) = delete;

        void             reset();
        Property*        get_property() const;
        Node*            get_node() const;
        VariableNode*    get_connected_variable() const;
        std::string      serialize_source() const;
        bool             has_slot( SlotFlag ) const;
        bool             has_input_connected() const;
        SlotView*        get_slot( SlotFlag ) const;
        ImVec2 get_pos( ID8<Slot> identifier ) const;
    };
}