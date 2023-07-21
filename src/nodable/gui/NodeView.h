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
#include "core/Slots.h"

#include "types.h"     // for constants and forward declarations

namespace ndbl
{
    // forward declaration
    class Node;
    class Graph;
    class NodeView;
    class PropertyView;
    class PropertyConnector;
    class NodeConnector;

	/** We use this enum to identify all NodeView detail modes */
	enum class NodeViewDetail: unsigned short int
	{
		Minimalist  = 0,
		Essential   = 1,
		Exhaustive  = 2,
		Default     = Essential
	};

    enum class ViewConstraint_t {
        AlignOnBBoxLeft,
        MakeRowAndAlignOnBBoxTop,
        MakeRowAndAlignOnBBoxBottom,
        FollowWithChildren,
    };

    R_ENUM(ViewConstraint_t)
    R_ENUM_VALUE(AlignOnBBoxLeft)
    R_ENUM_VALUE(MakeRowAndAlignOnBBoxTop)
    R_ENUM_VALUE(MakeRowAndAlignOnBBoxBottom)
    R_ENUM_VALUE(FollowWithChildren)
    R_ENUM_END

	/**
	 * A class to abstract a constraint between some NodeView
	 */
	class ViewConstraint {
	public:
	    using Filter = std::function<bool(ViewConstraint*)>;

	    ViewConstraint(const char* _name, ViewConstraint_t _type);
	    void apply(float _dt);
        void apply_when(const Filter& _lambda) { m_filter = _lambda; }
        void add_target(NodeView*);
        void add_driver(NodeView*);
        void add_targets(const std::vector<NodeView*>&);
        void add_drivers(const std::vector<NodeView*>&);
        void draw_view();

        ImVec2 m_offset; // offset applied to the constrain

        static const Filter no_target_expanded;
        static const Filter drivers_are_expanded;
        static const Filter always;

    private:
        const char*       m_name;
        bool              m_is_enable;
        Filter            m_filter; // Lambda returning true if this constrain should apply.
	    ViewConstraint_t  m_type;
        std::vector<NodeView*> m_drivers; // driving the targets
        std::vector<NodeView*> m_targets;
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
    class NodeView : public fw::View, public Component
	{
	public:
		NodeView();
		~NodeView();
        NodeView (const NodeView&) = delete;
        NodeView& operator= (const NodeView&) = delete;

        Slots<NodeView*>        successors;
        Slots<NodeView*>        children;
        Slots<NodeView*>        outputs;
        Slots<NodeView*>        inputs;
        bool                    pinned;               // When pinned, the view is not affected by ViewConstraints
        ImVec2                  position_offset_user; // User defined position offset to arrange graph without loosing automatic layout

		void                    set_owner(Node *_node)override;
		void                    expose(Property *);
		bool                    update(float);
        ImVec2                  get_position(fw::Space, bool round = false) const;
		void                    set_position(ImVec2, fw::Space);
		void                    translate(ImVec2, bool _recurse = false);
		void                    translate_to(ImVec2 desiredPos, float _factor, bool _recurse = false);
		void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        ImRect                  get_rect(bool _view = false, bool _ignorePinned = true
                                      , bool _ignoreMultiConstrained = true, bool _ignoreSelf = false) const;
        void                    add_constraint(ViewConstraint&);
        void                    apply_constraints(float _dt);
        void                    clear_constraints();
        const PropertyView*     get_property_view(const Property *)const;
        inline ImVec2           get_size() const { return m_size; }
        bool                    is_dragged()const { return s_dragged == this; }
        bool                    is_expanded()const { return m_expanded; }
        void                    add_force_to_translate_to(ImVec2 desiredPos, float _factor, bool _recurse = false);
        void                    add_force(ImVec2 force, bool _recurse = false);
        void                    apply_forces(float _dt, bool _recurse);
        void                    set_expanded_rec(bool _expanded);
        void                    set_expanded(bool _expanded);
        void                    set_inputs_visible(bool _visible, bool _recursive = false);
        void                    set_children_visible(bool _visible, bool _recursive = false);
        bool                    should_follow_output(const NodeView*) const;
        void                    expand_toggle();
        void                    expand_toggle_rec();
        void                    enable_edition(bool _enable = true) { m_edition_enable = _enable; }
        ImRect                  get_screen_rect();
        static ImRect           get_rect(
                                    const std::vector<const NodeView*>*,
                                    bool _recursive = false,
                                    bool _ignorePinned = true,
                                    bool _ignoreMultiConstrained = true); // rectangle is in local space
        static void             set_selected(NodeView*);
        static NodeView*        get_selected();
        static bool             is_selected(NodeView*);
        static void             start_drag(NodeView*);
        static bool		        is_any_dragged();
        static bool             is_inside(NodeView*, ImRect);
        static void             constraint_to_rect(NodeView*, ImRect);
        static NodeView*        get_dragged();
        static bool             draw_input(Property *_property, const char *_label);
        static void             draw_as_properties_panel(NodeView *_view, bool *_show_advanced);
        static void             set_view_detail(NodeViewDetail _viewDetail); // Change view detail globally
        static NodeViewDetail   get_view_detail() { return s_view_detail; }
        static NodeView*        substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive = true);
    private:
        bool                    draw_implem()override;
        bool                    draw_property(PropertyView *_view);
        bool                    is_exposed(const Property *_property)const;
        void                    update_labels_from_name(Node *_node);
        static void             DrawNodeRect(ImVec2 rect_min, ImVec2 rect_max, ImColor color, ImColor border_highlight_col, ImColor shadow_col, ImColor border_col, bool selected, float border_radius, float padding) ;

        std::string     m_label;
        std::string     m_short_label;
        bool            m_apply_constraints;
        bool            m_edition_enable;
        ImVec2          m_forces_sum;
        ImVec2          m_last_frame_forces_sum;
        bool            m_expanded;
		ImVec2          m_position;                      // local position (not affected by scroll)
		ImVec2          m_size;
		float           m_opacity;
		bool            m_force_property_inputs_visible;
		float           m_border_radius;
		ImColor         m_border_color_selected;
		std::vector<NodeConnector*>          m_predecessors;
		std::vector<NodeConnector*>          m_successors;
		std::vector<PropertyView*>             m_exposed_input_only_properties;
		std::vector<PropertyView*>             m_exposed_out_or_inout_properties;
        std::map<const Property *, PropertyView*> m_exposed_properties;
        PropertyView*                          m_exposed_this_property_view;
        std::vector<ViewConstraint>      m_constraints;

		static NodeView*              s_selected;
		static NodeView*              s_dragged;
        static const float            s_property_input_size_min;
        static const ImVec2           s_property_input_toggle_button_size;
        static std::vector<NodeView*> s_instances;
        static NodeViewDetail         s_view_detail;

        REFLECT_DERIVED_CLASS()
    };


    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
    public:
        ImVec2              m_position;
        Property *          m_property;
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
