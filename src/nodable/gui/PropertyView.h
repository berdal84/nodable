#pragma once

#include "imgui.h"

namespace ndbl {

    // forward declarations
    class Property;
    class PropertyConnector;
    class NodeView;

    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        ImVec2              m_position;
        Property*           m_property;
        NodeView*           m_nodeView;
        PropertyConnector*  m_in;
        PropertyConnector*  m_out;
        bool                m_showInput;
        bool                m_touched;

        PropertyView(Property * _property, NodeView* _nodeView);
        ~PropertyView();
        PropertyView (const PropertyView&) = delete;
        PropertyView& operator= (const PropertyView&) = delete;

        void reset()
        {
            m_touched   = false;
            m_showInput = false;
        }
    };
}