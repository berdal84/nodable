#pragma once

#include <string>
#include <map>
#include <cmath> // round()
#include <algorithm>
#include <observe/observer.h>
#include <imgui/imgui.h>   // for vec2

#include <nodable/Nodable.h>    // for constants and forward declarations
#include <nodable/View.h>       // base class
#include <nodable/Component.h>  // base class
#include <nodable/Member.h>
#include <nodable/Slots.h>
#include <nodable/Reflect.h>

namespace Nodable
{
    // forward declaration
    class Node;
    class GraphNode;
    class NodeView;
    class MemberView;
    class MemberConnector;
    class NodeConnector;
    class AppContext;

	/** We use this enum to identify all NodeView detail modes */
	enum class NodeViewDetail: unsigned short int
	{
		Minimalist  = 0,
		Essential   = 1,
		Exhaustive  = 2,
		Default     = Essential
	};

	/**
	 * A class to abstract a constraint between some NodeView
	 */
	class NodeViewConstraint {
	public:
	    enum Type {
	        AlignOnBBoxTop,
	        AlignOnBBoxLeft,
	        MakeRowAndAlignOnBBoxTop,
	        MakeRowAndAlignOnBBoxBottom,
	        FollowWithChildren,
	        Follow,
        };

	    NodeViewConstraint(const AppContext* _ctx, Type _type);
	    void apply(float _dt);
	    void addSlave(NodeView*);
	    void addMaster(NodeView*);
        void addSlaves(const std::vector<NodeView *> &vector);
        void addMasters(const std::vector<NodeView *> &vector);
        vec2 m_offset;

    private:
        const AppContext* m_context;
	    Type type;
        std::vector<NodeView*> masters;
	    std::vector<NodeView*> slaves;
    };

	/**
	 * This class implement a view for Nodes using ImGui.
	 */
	class NodeView :  public Component, public View
	{
	public:
		NodeView(AppContext* _ctx);
		~NodeView();
        NodeView (const NodeView&) = delete;
        NodeView& operator= (const NodeView&) = delete;

		observe::Observer m_nodeRelationAddedObserver;
		observe::Observer m_nodeRelationRemovedObserver;

		/** override Component::setOwner(Node*) to extract some information from owner before to actually set it */
		void set_owner(Node *_node)override;

		void exposeMember(Member* );

		/** Draw the view at its position into the current window
		   Returns true if nod has been edited, false either */
		bool draw ()override;

		/** Should be called once per frame to update the view */
		bool update()override;

		inline const vec2& getPos()const { return m_position; }
		inline vec2 getPosRounded()const { return vec2(std::round(m_position.x), std::round(m_position.y)); }

		/** Set a new position (top-left corner relative) vector to this view */
		void  setPosition(vec2);

		/** Apply a translation vector to the view's position */
		void translate(vec2, bool _recurse = false);

		void translateTo(vec2 desiredPos, float _factor, bool _recurse = false);
		/** Arrange input nodes recursively while keeping this node position unchanged.
		 *  Note: Some input connected Nodes can stay fixed if they are pinned. */
		void arrangeRecursively(bool _smoothly = true);

		/** Draw advance properties (components, dirty state, etc.) */
		void drawAdvancedProperties();

		/** Get the node label
		 * Note: depends on s_viewDetail, can be just an ICON_FA (4 char) or an ICON_FA + a label */
        std::string getLabel();

        /** Compute the bounding rectangle of this view */
        ImRect getRect(bool _recursive = false, bool _ignorePinned = true, bool _ignoreMultiConstrained = true, bool _ignoreSelf = false);

        void addConstraint(NodeViewConstraint &_constraint);
        void applyConstraints(float _dt);
        void clearConstraints();

		/** Set a NodeView as selected.
		 * Note: Only a single view can be selected at the same time */
		static void SetSelected(NodeView*);

		/** Return a pointer to the selected view or nullptr if no view are selected */
		static NodeView* GetSelected();

		/** Return true if the given NodeView is selected */
		static bool       IsSelected(NodeView*);

		/** Start to drag a NodeView. Only a single view can be dragged at the same time. */
		static void       StartDragNode(NodeView*);

		/** Return true if any NodeView is currently dragged */
		static bool		  IsAnyDragged();

		/** Return true if a given NodeView is contained by a given Rectangle */
		static bool       IsInsideRect(NodeView*, ImRect);

		/** Move instantly a given NodeView to be inside a given Rectangle */
		static void       ConstraintToRect(NodeView*, ImRect);

		/** Return a pointer to the dragged NodeView or nullptr if no view are dragged */
		static NodeView*  GetDragged();

		/**
		 * Draw the ImGui input to modify a given Member.
		 *
		 * @param _member
		 * @param _label by default nullptr, you can override it to change input label.
		 * @return
		 */
        static bool DrawMemberInput(Member *_member, const char* _label = nullptr);

        /**
         * Draw a NodeView as a properties panel.
         * All input, output, components, parent container and other information are draw in a column.
         * @param _view
         */
        static void DrawNodeViewAsPropertiesPanel(NodeView *_view);

        /** set globally the draw detail of nodes */
        static NodeViewDetail s_viewDetail;

        /** Change view detail globally */
        static void SetDetail(NodeViewDetail _viewDetail);

        /** Get a MemberView given a Member */
        const MemberView*       getMemberView(const Member* _member)const;
        inline vec2           getSize() const { return m_size; }
        vec2                  getScreenPos();
        inline void             setPinned(bool b) { m_pinned = b; }
        inline bool             isPinned()const { return m_pinned; }
        inline bool             isDragged() { return s_draggedNode == this; }
        static ImRect           GetRect(const std::vector<NodeView *>&, bool _recursive = false, bool _ignorePinned = true, bool _ignoreMultiConstrained = true);
        void                    addForceToTranslateTo(vec2 desiredPos, float _factor, bool _recurse = false);
        void                    addForce(vec2 force, bool _recurse = false);
        void                    applyForces(float _dt, bool _recurse);
        void                    setChildrenVisible(bool _visible, bool _recursive = false);
        void                    setInputsVisible(bool _visible, bool _recursive = false);
        bool                    shouldFollowOutput(const NodeView*);
        Slots<NodeView*>&       successor_slots() { return m_successor_slots; }
        Slots<NodeView*>&       children_slots() { return m_children_slots; }
        Slots<NodeView*>&       output_slots() { return m_output_slots; }
        Slots<NodeView*>&       input_slots() { return m_input_slots; }
        void                    toggleExpansion();
    private:
        virtual bool    update(float _deltaTime);
		bool            drawMemberView(MemberView *_view);
        void            drawMemberViewConnector(MemberView* _view, Way _way, float _connectorRadius);
        bool            isMemberExposed(const Member *_member)const;

        vec2          m_forces_sum;
        vec2          m_last_frame_forces_sum;
        bool            m_childrenVisible;
		vec2          m_position;
		vec2          m_size;
		float           m_opacity;
		bool            m_forceMemberInputVisible;
		bool            m_pinned;
		float           m_borderRadius;
		ImColor         m_borderColorSelected;
        Slots<NodeView*> m_children_slots;
        Slots<NodeView*> m_input_slots;
        Slots<NodeView*> m_output_slots;
        Slots<NodeView*> m_successor_slots;
		std::vector<NodeConnector*>          m_predecessors_node_connnectors;
		std::vector<NodeConnector*>          m_successors_node_connectors;
		std::vector<MemberView*>             m_exposedInputOnlyMembers;
		std::vector<MemberView*>             m_exposedOutOrInOutMembers;
        std::map<const Member*, MemberView*> m_exposedMembers;
        MemberView*                          m_exposed_this_member_view;
        std::vector<NodeViewConstraint>      m_constraints;
        AppContext* m_context;

		static NodeView*              s_selected;
		static NodeView*              s_draggedNode;
        static const float            s_memberInputSizeMin;
        static const vec2           s_memberInputToggleButtonSize;
        static std::vector<NodeView*> s_instances;

        // Reflect this class
        REFLECT_DERIVED(NodeView)
        REFLECT_EXTENDS(View)
        REFLECT_EXTENDS(Component)
        REFLECT_END
    };


    /**
 * Simple struct to store a member view state
 */
    class MemberView
    {
        vec2            m_relative_pos;

    public:
        Member*           m_member;
        NodeView*         m_nodeView;
        MemberConnector*  m_in;
        MemberConnector*  m_out;
        bool              m_showInput;
        bool              m_touched;

        MemberView(const AppContext* _ctx, Member* _member, NodeView* _nodeView);
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
