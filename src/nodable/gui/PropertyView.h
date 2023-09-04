#pragma once

#include "imgui.h"
#include "fw/core/Pool.h"

namespace ndbl {

    // forward declarations
    class Property;
    class SlotView;
    class NodeView;
    using fw::pool::ID;

    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        u8_t                property;
        ImVec2              position;
        bool                show_input;
        bool                touched;
        const ID<NodeView>  node_view;

        PropertyView(u8_t _property_id, ID<NodeView> _node_view_id);
        ~PropertyView();
        PropertyView (const PropertyView&) = delete;
        PropertyView& operator= (const PropertyView&) = delete;

        void reset();
    private:
    };
}