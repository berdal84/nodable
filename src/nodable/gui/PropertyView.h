#pragma once

#include "imgui.h"
#include "fw/core/Pool.h"

namespace ndbl {

    // forward declarations
    class Property;
    class PropertyConnector;
    class NodeView;
    using fw::pool::ID;

    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        ImVec2              position;
        bool                show_input;
        bool                touched;
        const ID<NodeView> node_view;

        PropertyView(Property *_property, ID<NodeView> _nodeView);
        ~PropertyView();
        PropertyView (const PropertyView&) = delete;
        PropertyView& operator= (const PropertyView&) = delete;

        void                reset();
        Property*           property() { return m_property; }
        PropertyConnector*  input() const { return m_input; }
        PropertyConnector*  output() const { return m_output; }
    private:
        Property*           m_property;
        PropertyConnector*  m_input;
        PropertyConnector*  m_output;
    };
}