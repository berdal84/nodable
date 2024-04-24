#pragma once

#include <string>
#include <map>
#include <unordered_map>
#include <cmath> // round()
#include <algorithm>
#include <observe/observer.h>

#include "fw/gui/ImGuiEx.h"
#include "fw/gui/View.h"
#include "core/Component.h" // base class
#include "core/Property.h"
#include "Config.h"
#include "PropertyView.h"
#include "SlotView.h"
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
        bool                    draw()override;
		void                    set_owner(PoolID<Node>)override;
        bool                    update(float);
        ImVec2                  get_position() const { return m_position; };
        ImVec2                  get_position(fw::Space, bool round = false) const;
		void                    set_position(ImVec2, fw::Space);
		void                    translate(ImVec2, bool _recurse = false);
		void                    translate_to(ImVec2 desiredPos, float _factor, bool _recurse = false);
		void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        ImRect                  get_rect(bool _recursively = false, bool _ignorePinned = true, bool _ignoreMultiConstrained = true, bool _ignoreSelf = false) const;
        const PropertyView*     get_property_view( ID<Property> _id )const;
        inline ImVec2           get_size() const { return m_size; }
        bool                    is_dragged()const;
        bool                    is_expanded()const { return m_expanded; }
        void                    set_expanded_rec(bool _expanded);
        void                    set_expanded(bool _expanded);
        void                    set_inputs_visible(bool _visible, bool _recursive = false);
        void                    set_children_visible(bool _visible, bool _recursive = false);
        void                    expand_toggle();
        void                    expand_toggle_rec();
        void                    enable_edition(bool _enable = true) { m_edition_enable = _enable; }
        ImRect                  get_screen_rect() const;
        static ImRect           get_rect(
                const std::vector<NodeView *> &_views,
                bool _recursive = false,
                bool _ignorePinned = true,
                bool _ignoreMultiConstrained = true); // rectangle is in local space
        static void             set_selected(PoolID<NodeView>);
        static PoolID<NodeView> get_selected();
        static bool             is_selected(PoolID<NodeView>);
        static bool		        is_any_dragged();
        static bool             is_any_selected();
        static bool             is_inside(NodeView*, ImRect);
        static void             constraint_to_rect(NodeView*, ImRect);
        static PoolID<NodeView> get_dragged();
        static bool             draw_property_view(PropertyView*, const char* _override_label);
        static void             draw_as_properties_panel(NodeView* _view, bool *_nodes );
        static void             set_view_detail(NodeViewDetail _viewDetail); // Change view detail globally
        static NodeViewDetail   get_view_detail() { return s_view_detail; }
        static NodeView*        substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive = true);
        static std::vector<NodeView*> substitute_with_parent_if_not_visible(const std::vector<NodeView*>& _in, bool _recurse = true );
        ImVec2                  get_slot_pos( const Slot& );
        ImRect                  get_slot_rect( const Slot& _slot, const Config& _config, i8_t _count ) const;
        ImRect                  get_slot_rect( const SlotView &_slot_view, const Config &_config, i8_t _pos ) const;
        ImVec2                  get_slot_normal( const Slot& slot) const;
        void                    set_color( const ImVec4* _color, ColorType _type = Color_FILL );
        ImColor                 get_color(ColorType _type) const;
        static bool             none_is_visible( std::vector<NodeView*> vector1 );

    private:

        void                    set_adjacent_visible(SlotFlags flags, bool _visible, bool _recursive);
        bool                    _draw_property_view(PropertyView* _view);
        void                    update_labels_from_name(const Node *_node);
        static void DrawNodeRect( ImVec2 rect_min, ImVec2 rect_max, ImColor color, ImColor border_highlight_col, ImColor shadow_col, ImColor border_col, bool selected, float border_radius, float border_width );

        std::string     m_label;
        std::string     m_short_label;
        bool            m_edition_enable;
        bool            m_expanded;
		ImVec2          m_position; // local position (not affected by scroll)
		ImVec2          m_size;
        bool            m_pinned;
		float           m_opacity;
        std::array<const ImVec4*, Color_COUNT>           m_colors;
        std::vector<SlotView>                            m_slot_views;
        std::vector<PropertyView>                        m_property_views;
        PropertyView*                                    m_property_view_this;
        std::vector<PropertyView*>                       m_property_views_with_input_only;
        std::vector<PropertyView*>                       m_property_views_with_output_or_inout;
		static PoolID<NodeView>                          s_selected;
		static PoolID<NodeView>                          s_dragged;
        static const float                               s_property_input_size_min;
        static const ImVec2                              s_property_input_toggle_button_size;
        static NodeViewDetail                            s_view_detail;

        REFLECT_DERIVED_CLASS()
    };
}
