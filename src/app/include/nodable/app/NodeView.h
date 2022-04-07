#pragma once

#include <string>
#include <map>
#include <cmath> // round()
#include <algorithm>
#include <observe/observer.h>
#include <imgui/imgui.h>   // for vec2

#include <nodable/app/types.h>    // for constants and forward declarations
#include <nodable/app/View.h>       // base class
#include <nodable/core/Component.h>  // base class
#include <nodable/core/Member.h>
#include <nodable/core/Slots.h>
#include <nodable/core/reflection/R.h>

namespace Nodable
{
    // forward declaration
    class Node;
    class GraphNode;
    class NodeView;
    class MemberView;
    class MemberConnector;
    class NodeConnector;
    class IAppCtx;

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

	    using Filter = std::function<bool(ViewConstraint*)>;

	    ViewConstraint(IAppCtx& _ctx, const char* _name, ViewConstraint_t _type);
	    void apply(float _dt);
        void apply_when(const Filter& _lambda) { m_filter = _lambda; }
        void add_target(NodeView*);
        void add_driver(NodeView*);
        void add_targets(const NodeViewVec&);
        void add_drivers(const NodeViewVec&);
        void draw_view();

        vec2 m_offset;

        static const Filter no_target_expanded;
        static const Filter drivers_are_expanded;
        static const Filter always;

    private:
        bool              m_is_enable;
        bool              should_apply();
        Filter            m_filter;
        IAppCtx&          m_ctx;
	    ViewConstraint_t  m_type;
        NodeViewVec       m_drivers;
        NodeViewVec       m_targets;
        const char*       m_name;
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
	class NodeView :  public Component, public View
	{
	public:
		NodeView(IAppCtx& _ctx);
		~NodeView();
        NodeView (const NodeView&) = delete;
        NodeView& operator= (const NodeView&) = delete;

		observe::Observer m_nodeRelationAddedObserver;
		observe::Observer m_nodeRelationRemovedObserver;

		void                    set_owner(Node *_node)override;
		void                    expose(Member*);
		bool                    draw()override;
		bool                    update()override;
		inline const vec2&      get_position()const { return m_position; }
		inline vec2             get_position_rounded()const { return vec2(std::round(m_position.x), std::round(m_position.y)); }
		void                    set_position(vec2);
		void                    translate(vec2, bool _recurse = false);
		void                    translate_to(vec2 desiredPos, float _factor, bool _recurse = false);
		void                    arrange_recursively(bool _smoothly = true);
        std::string             get_label();
        ImRect                  get_rect(bool _view = false, bool _ignorePinned = true
                                      , bool _ignoreMultiConstrained = true, bool _ignoreSelf = false);
        void                    add_constraint(ViewConstraint&);
        void                    apply_constraints(float _dt);
        void                    clear_constraints();
        const MemberView*       get_member_view(const Member*)const;
        inline vec2             get_size() const { return m_size; }
        vec2                    get_screen_position();
        void                    set_pinned(bool b) { m_pinned = b; }
        bool                    is_pinned()const { return m_pinned; }
        bool                    is_dragged()const { return s_dragged == this; }
        bool                    is_expanded()const { return m_expanded; }
        void                    add_force_to_translate_to(vec2 desiredPos, float _factor, bool _recurse = false);
        void                    add_force(vec2 force, bool _recurse = false);
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
        static bool             draw_input(IAppCtx &_ctx, Member *_member, const char *_label);
        static void             draw_as_properties_panel(IAppCtx &_ctx, NodeView *_view, bool *_show_advanced);
        static void             set_view_detail(NodeViewDetail _viewDetail); // Change view detail globally
        static NodeViewDetail   get_view_detail() { return s_view_detail; }
        static NodeView*        substitute_with_parent_if_not_visible(NodeView* _view, bool _recursive = true);

    private:
        virtual bool            update(float _deltaTime);
		bool                    draw(MemberView *_view);
        bool                    is_exposed(const Member *_member)const;

        bool            m_apply_constraints;
        bool            m_edition_enable;
        vec2            m_forces_sum;
        vec2            m_last_frame_forces_sum;
        bool            m_expanded;
		vec2            m_position;
		vec2            m_size;
		float           m_opacity;
		bool            m_force_member_inputs_visible;
		bool            m_pinned;
		float           m_border_radius;
		ImColor         m_border_color_selected;
        Slots<NodeView*> m_children_slots;
        Slots<NodeView*> m_input_slots;
        Slots<NodeView*> m_output_slots;
        Slots<NodeView*> m_successor_slots;
		std::vector<NodeConnector*>          m_predecessors;
		std::vector<NodeConnector*>          m_successors;
		std::vector<MemberView*>             m_exposed_input_only_members;
		std::vector<MemberView*>             m_exposed_out_or_inout_members;
        std::map<const Member*, MemberView*> m_exposed_members;
        MemberView*                          m_exposed_this_member_view;
        std::vector<ViewConstraint>      m_constraints;

		static NodeView*              s_selected;
		static NodeView*              s_dragged;
        static const float            s_member_input_size_min;
        static const vec2             s_member_input_toggle_button_size;
        static std::vector<NodeView*> s_instances;
        static NodeViewDetail         s_view_detail;

        // Reflect this class
        R_DERIVED(NodeView)
        R_EXTENDS(View)
        R_EXTENDS(Component)
        R_END
    };


    /**
     * Simple struct to store a member view state
     */
    class MemberView
    {
        vec2              m_relative_pos;

    public:
        Member*           m_member;
        NodeView*         m_nodeView;
        MemberConnector*  m_in;
        MemberConnector*  m_out;
        bool              m_showInput;
        bool              m_touched;

        MemberView(IAppCtx& _ctx, Member* _member, NodeView* _nodeView);
        ~MemberView();
        MemberView (const MemberView&) = delete;
        MemberView& operator= (const MemberView&) = delete;
        /**
         * Reset the view
         */
        void reset()
        {
            m_touched = false;
            m_showInput = false;
        }

        vec2 relative_pos() const { return m_relative_pos; }
        void relative_pos(vec2 _pos) { m_relative_pos = _pos; }
    };
}
