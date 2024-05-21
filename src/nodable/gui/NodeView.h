#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <cmath> // round()
#include <algorithm>
#include <observe/observer.h>

#include "Config.h"
#include "PropertyView.h"
#include "SlotView.h"
#include "nodable/core/Component.h"// base class
#include "nodable/core/Property.h"
#include "fw/core/geometry/Box2D.h"
#include "fw/gui/ImGuiEx.h"
#include "fw/gui/View.h"
#include "types.h"

namespace ndbl
{
    // forward declaration
    class Node;
    class Graph;
    class NodeView;
    class Slot;
    class SlotView;
    class NodeViewConstraint;

	/** We use this enum to identify all NodeView detail modes */
	enum class NodeViewDetail: unsigned short int
	{
		Minimalist  = 0,
		Essential   = 1,
		Exhaustive  = 2,
		Default     = Essential
	};

    /**
     * Enum to define some color types
     */
    enum ColorType
    {
        Color_FILL,
        Color_COUNT
    };

    typedef int NodeViewFlags;
    enum NodeViewFlag_
    {
        NodeViewFlag_NONE                    = 0,
        NodeViewFlag_RECURSIVELY             = 1 << 0,
        NodeViewFlag_IGNORE_PINNED           = 1 << 1,
        NodeViewFlag_IGNORE_MULTICONSTRAINED = 1 << 2,
        NodeViewFlag_IGNORE_SELF             = 1 << 3,
        NodeViewFlag_IGNORE_HIDDEN           = 1 << 4
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
    class NodeView : public Component, public fw::View
	{
	public:
		NodeView();
		~NodeView();
        NodeView (NodeView&&) = default;
        NodeView& operator=(NodeView&&) = default;

        std::vector<PoolID<NodeView>> get_adjacent(SlotFlags) const;
        void                    pinned(bool b) { m_pinned = b; }
        bool                    pinned() const { return m_pinned; }
        bool                    onDraw()override;
		void                    set_owner(PoolID<Node>)override;
        bool                    update(float);
		void                    translate(fw::Vec2, NodeViewFlags flags );
		void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        fw::Rect                get_rect(fw::Space, NodeViewFlags = NodeViewFlag_IGNORE_PINNED | NodeViewFlag_IGNORE_MULTICONSTRAINED) const;
        const PropertyView*     get_property_view( ID<Property> _id )const;
        inline fw::Vec2         get_size() const { return box.size(); }
        bool                    is_dragged()const;
        bool                    is_expanded()const { return m_expanded; }
        void                    set_expanded_rec(bool _expanded);
        void                    set_expanded(bool _expanded);
        void                    set_inputs_visible(bool _visible, bool _recursive = false);
        void                    set_children_visible(bool _visible, bool _recursive = false);
        void                    expand_toggle();
        void                    expand_toggle_rec();
        void                    enable_edition(bool _enable = true) { m_edition_enable = _enable; }
        static fw::Rect         get_rect(const std::vector<NodeView *> &_views, fw::Space, NodeViewFlags = NodeViewFlag_NONE );
        static std::vector<fw::Rect>   get_rects( const std::vector<NodeView*>& _in_views, fw::Space space, NodeViewFlags flags = NodeViewFlag_NONE );
        static void             set_selected(PoolID<NodeView>);
        static PoolID<NodeView> get_selected();
        static bool             is_selected(PoolID<NodeView>);
        static bool		        is_any_dragged();
        static bool             is_any_selected();
        static bool             is_inside(NodeView*, fw::Rect, fw::Space);
        static void             constraint_to_rect(NodeView*, fw::Rect );
        static PoolID<NodeView> get_dragged();
        static bool             draw_property_view(PropertyView*, const char* _override_label);
        static void             draw_as_properties_panel(NodeView* _view, bool *_nodes );
        static void             set_view_detail(NodeViewDetail _viewDetail); // Change view detail globally
        static NodeViewDetail   get_view_detail() { return s_view_detail; }
        static NodeView*        substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive = true);
        static std::vector<NodeView*> substitute_with_parent_if_not_visible(const std::vector<NodeView*>& _in, bool _recurse = true );
        fw::Vec2                get_slot_pos( const Slot& );
        fw::Rect                get_slot_rect( const Slot& _slot, const Config& _config, i8_t _count ) const;
        fw::Rect                get_slot_rect( const SlotView &_slot_view, const Config &_config, i8_t _pos ) const;
        fw::Vec2                get_slot_normal( const Slot& slot) const;
        void                    set_color( const fw::Vec4* _color, ColorType _type = Color_FILL );
        fw::Vec4                get_color(ColorType _type) const;

        static bool             none_is_visible( std::vector<NodeView*> vector1 );

    private:
        void                    set_adjacent_visible(SlotFlags flags, bool _visible, bool _recursive);
        bool                    _draw_property_view(PropertyView* _view);
        void                    update_labels_from_name(const Node *_node);

        static void DrawNodeRect(
            fw::Rect rect,
            fw::Vec4 color,
            fw::Vec4 border_highlight_col,
            fw::Vec4 shadow_col,
            fw::Vec4 border_col,
            bool selected,
            float border_radius,
            float border_width
        );
        std::string     m_label;
        std::string     m_short_label;
        bool            m_edition_enable;
        bool            m_expanded;
        bool            m_pinned;
        float           m_opacity;
        std::array<const fw::Vec4*, Color_COUNT> m_colors;
        std::vector<SlotView>      m_slot_views;
        std::vector<PropertyView>  m_property_views;
        PropertyView*              m_property_view_this;
        std::vector<PropertyView*> m_property_views_with_input_only;
        std::vector<PropertyView*> m_property_views_with_output_or_inout;
        static PoolID<NodeView>    s_selected;
        static PoolID<NodeView>    s_dragged;
        static const float         s_property_input_size_min;
        static const fw::Vec2      s_property_input_toggle_button_size;

        static NodeViewDetail      s_view_detail;
    REFLECT_DERIVED_CLASS()
    };
}
