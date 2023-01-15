#pragma once

#include <memory> // std::shared_ptr

#include <nodable/app/Connector.h>
#include <nodable/app/types.h>
#include <nodable/core/reflection/reflection>
#include <nodable/core/assertions.h>

namespace ndbl {

    // forward declarations
    class PropertyView;
    class Property;
    class IAppCtx;

    /**
     * @brief A PropertyConnector represents a physical input or output on a PropertyView.
     */
    class PropertyConnector: public Connector<PropertyConnector>
    {
    public:

        enum class Side
        {
            Top,
            Bottom,
            Left,
            Right
        };

        PropertyConnector(IAppCtx& _ctx, PropertyView* _property, Way _way, Side _pos)
            : m_ctx(_ctx)
            , m_propertyView(_property)
            , m_way(_way)
            , m_display_side(_pos)
        {
            NDBL_ASSERT(_property)
        };

        ~PropertyConnector() = default;
        PropertyConnector (const PropertyConnector&) = delete;
        PropertyConnector& operator= (const PropertyConnector&) = delete;

        Property *            get_property()const;
        type        get_property_type()const;
        vec2               get_pos()const override;
        bool               share_parent_with(const PropertyConnector *other)const override;
        bool               has_node_connected() const;
        static void        draw(const PropertyConnector *_connector, float _radius, const ImColor &_color, const ImColor &_borderColor, const ImColor &_hoverColor, bool _editable);
        static void        dropped(const PropertyConnector *_left, const PropertyConnector *_right);

        PropertyView* m_propertyView;
        Side        m_display_side;
        Way         m_way;
        IAppCtx&    m_ctx;
        static const PropertyConnector* s_hovered;
        static const PropertyConnector* s_dragged;
        static const PropertyConnector* s_focused;


    };
}