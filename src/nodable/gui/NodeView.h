#pragma once

#include <string>
#include <map>
#include <cmath> // round()
#include <algorithm>
#include <observe/observer.h>

#include "fw/gui/ImGuiEx.h"
#include "fw/gui/View.h"

#include "core/Component.h" // base class
#include "core/Property.h"
#include "core/SlotBag.h"

#include "types.h"     // for constants and forward declarations

namespace ndbl
{
    // forward declaration
    class Node;
    class Graph;
    class NodeView;
    class PropertyView;
    class PropertyConnectorView;
    class NodeConnector;
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
	 * This class implement a view for Nodes using ImGui.
	 */
    class NodeView : public Component, public fw::View
	{
	public:
		NodeView();
		~NodeView();
        NodeView (NodeView&&) = default;
        NodeView& operator=(NodeView&&) = default;

        std::vector<ID<NodeView>> successors;
        std::vector<ID<NodeView>> children;
        std::vector<ID<NodeView>> outputs;
        std::vector<ID<NodeView>> inputs;

        bool                pinned;

        bool                    draw()override;
		void                    set_owner(ID<Node>)override;
		void                    expose(Property* );
		bool                    update(float);
        ImVec2                  get_position() const { return m_position; };
        ImVec2                  get_position(fw::Space, bool round = false) const;
		void                    set_position(ImVec2, fw::Space);
		void                    translate(ImVec2, bool _recurse = false);
		void                    translate_to(ImVec2 desiredPos, float _factor, bool _recurse = false);
		void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        ImRect                  get_rect(bool _view = false, bool _ignorePinned = true
                                      , bool _ignoreMultiConstrained = true, bool _ignoreSelf = false) const;
        const PropertyView*     get_property_view(const Property * _property)const;
        inline ImVec2           get_size() const { return m_size; }
        bool                    is_dragged()const { return s_dragged == id(); }
        bool                    is_expanded()const { return m_expanded; }
        void                    set_expanded_rec(bool _expanded);
        void                    set_expanded(bool _expanded);
        void                    set_inputs_visible(bool _visible, bool _recursive = false);
        void                    set_children_visible(bool _visible, bool _recursive = false);
        bool                    should_follow_output(ID<const NodeView>) const;
        void                    expand_toggle();
        void                    expand_toggle_rec();
        void                    enable_edition(bool _enable = true) { m_edition_enable = _enable; }
        ImRect                  get_screen_rect();
        static ImRect           get_rect(
                const std::vector<NodeView *> &_views,
                bool _recursive = false,
                bool _ignorePinned = true,
                bool _ignoreMultiConstrained = true); // rectangle is in local space
        static void             set_selected(ID<NodeView>);
        static ID<NodeView>     get_selected();
        static bool             is_selected(ID<NodeView>);
        static void             start_drag(ID<NodeView>);
        static bool		        is_any_dragged();
        static bool             is_any_selected();
        static bool             is_inside(NodeView*, ImRect);
        static void             constraint_to_rect(NodeView*, ImRect);
        static ID<NodeView>     get_dragged();
        static bool             draw_property(Property* _property, const char *_label);
        static void             draw_as_properties_panel(NodeView* _view, bool *_show_advanced);
        static void             set_view_detail(NodeViewDetail _viewDetail); // Change view detail globally
        static NodeViewDetail   get_view_detail() { return s_view_detail; }
        static NodeView*        substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive = true);
    private:
        bool                    draw_property_view(PropertyView* _view);
        bool                    is_exposed(const Property *_property)const;
        void                    update_labels_from_name(const Node *_node);
        static void             DrawNodeRect(ImVec2 rect_min, ImVec2 rect_max, ImColor color, ImColor border_highlight_col, ImColor shadow_col, ImColor border_col, bool selected, float border_radius, float padding) ;

        std::string     m_label;
        std::string     m_short_label;
        bool            m_edition_enable;
        bool            m_expanded;
		ImVec2          m_position;                      // local position (not affected by scroll)
		ImVec2          m_size;
		float           m_opacity;
		ImColor         m_border_color_selected;
		std::vector<NodeConnector*> m_predecessors;
		std::vector<NodeConnector*> m_successors;
		std::vector<PropertyView*>  m_exposed_input_only_properties;
		std::vector<PropertyView*>  m_exposed_out_or_inout_properties;
        PropertyView*               m_exposed_this_property_view;
        std::map<const Property*, PropertyView*> m_exposed_properties;

		static ID<NodeView>       s_selected;
		static ID<NodeView>       s_dragged;
        static const float            s_property_input_size_min;
        static const ImVec2           s_property_input_toggle_button_size;
        static NodeViewDetail         s_view_detail;

        REFLECT_DERIVED_CLASS()
    };
}
