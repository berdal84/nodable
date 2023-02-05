#pragma once

#include <string>
#include <map>
#include <cmath> // round()
#include <algorithm>
#include <observe/observer.h>
#include <imgui/imgui.h>   // for vec2
#include <fw/gui/View.h>

#include <ndbl/gui/types.h>     // for constants and forward declarations
#include <ndbl/core/Component.h>// base class
#include <ndbl/core/Property.h>
#include <ndbl/core/Slots.h>

namespace ndbl
{
    // forward declaration
    class Node;
    class GraphNode;
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
        AlignOnBBoxTop,
        AlignOnBBoxLeft,
        MakeRowAndAlignOnBBoxTop,
        MakeRowAndAlignOnBBoxBottom,
        FollowWithChildren,
        Follow,
    };

    R_ENUM(ViewConstraint_t)
    R_ENUM_VALUE(AlignOnBBoxTop)
    R_ENUM_VALUE(AlignOnBBoxLeft)
    R_ENUM_VALUE(MakeRowAndAlignOnBBoxTop)
    R_ENUM_VALUE(MakeRowAndAlignOnBBoxBottom)
    R_ENUM_VALUE(FollowWithChildren)
    R_ENUM_VALUE(Follow)
    R_ENUM_END

	/**
	 * A class to abstract a constraint between some NodeView
	 */
	class ViewConstraint {
	public:
        using NodeViews = std::vector<NodeView*>;
	    using Filter    = std::function<bool(ViewConstraint*)>;

	    ViewConstraint(const char* _name, ViewConstraint_t _type);
	    void apply(float _dt);
        void apply_when(const Filter& _lambda) { m_filter = _lambda; }
        void add_target(NodeView*);
        void add_driver(NodeView*);
        void add_targets(const NodeViews&);
        void add_drivers(const NodeViews&);
        void draw_view();

        fw::vec2 m_offset;

        static const Filter no_target_expanded;
        static const Filter drivers_are_expanded;
        static const Filter always;

    private:
        bool              m_is_enable;
        bool              should_apply();
        Filter            m_filter;
	    ViewConstraint_t  m_type;
        NodeViews         m_drivers;
        NodeViews         m_targets;
        const char*       m_name;
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
    class NodeView :  public Component, public fw::View
	{
	public:
		NodeView();
		~NodeView();
        NodeView (const NodeView&) = delete;
        NodeView& operator= (const NodeView&) = delete;

		observe::Observer m_onRelationAddedObserver;
		observe::Observer m_onRelationRemovedObserver;

		void                    set_owner(Node *_node)override;
		void                    expose(Property *);
		bool                    draw()override;
		bool                    update()override;
		inline const fw::vec2&  get_position()const { return m_position; }
		inline fw::vec2         get_position_rounded()const { return fw::vec2(std::round(m_position.x), std::round(m_position.y)); }
		void                    set_position(fw::vec2);
		void                    translate(fw::vec2, bool _recurse = false);
		void                    translate_to(fw::vec2 desiredPos, float _factor, bool _recurse = false);
		void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        ImRect                  get_rect(bool _view = false, bool _ignorePinned = true
                                      , bool _ignoreMultiConstrained = true, bool _ignoreSelf = false);
        void                    add_constraint(ViewConstraint&);
        void                    apply_constraints(float _dt);
        void                    clear_constraints();
        const PropertyView*     get_property_view(const Property *)const;
        inline fw::vec2         get_size() const { return m_size; }
        fw::vec2                get_screen_position();
        void                    set_pinned(bool b) { m_pinned = b; }
        bool                    is_pinned()const { return m_pinned; }
        bool                    is_dragged()const { return s_dragged == this; }
        bool                    is_expanded()const { return m_expanded; }
        void                    add_force_to_translate_to(fw::vec2 desiredPos, float _factor, bool _recurse = false);
        void                    add_force(fw::vec2 force, bool _recurse = false);
        void                    apply_forces(float _dt, bool _recurse);
        void                    set_expanded_rec(bool _expanded);
        void                    set_expanded(bool _expanded);
        void                    set_inputs_visible(bool _visible, bool _recursive = false);
        void                    set_children_visible(bool _visible, bool _recursive = false);
        bool                    should_follow_output(const NodeView*);
        Slots<NodeView*>&       successors() { return m_successor_slots; }
        Slots<NodeView*>&       children() { return m_children_slots; }
        Slots<NodeView*>&       outputs() { return m_output_slots; }
        Slots<NodeView*>&       inputs() { return m_input_slots; }
        void                    expand_toggle();
        void                    expand_toggle_rec();
        void                    enable_edition(bool _enable = true) { m_edition_enable = _enable; }

        static ImRect           get_rect(const std::vector<NodeView *>&, bool _recursive = false
                                        , bool _ignorePinned = true, bool _ignoreMultiConstrained = true);
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
        virtual bool            update(float _deltaTime);
		bool                    draw(PropertyView *_view);
        bool                    is_exposed(const Property *_property)const;

        std::string     m_label;
        std::string     m_short_label;
        bool            m_apply_constraints;
        bool            m_edition_enable;
        fw::vec2        m_forces_sum;
        fw::vec2        m_last_frame_forces_sum;
        bool            m_expanded;
		fw::vec2        m_position;
		fw::vec2        m_size;
		float           m_opacity;
		bool            m_force_property_inputs_visible;
		bool            m_pinned;
		float           m_border_radius;
		ImColor         m_border_color_selected;
        Slots<NodeView*> m_children_slots;
        Slots<NodeView*> m_input_slots;
        Slots<NodeView*> m_output_slots;
        Slots<NodeView*> m_successor_slots;
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
        static const fw::vec2         s_property_input_toggle_button_size;
        static std::vector<NodeView*> s_instances;
        static NodeViewDetail         s_view_detail;

        REFLECT_DERIVED_CLASS()

    };


    /**
     * Simple struct to store a property view state
     */
    class PropertyView
    {
        fw::vec2            m_relative_pos;

    public:
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

        fw::vec2  relative_pos() const { return m_relative_pos; }
        void      relative_pos(fw::vec2 _pos) { m_relative_pos = _pos; }
    };
}
