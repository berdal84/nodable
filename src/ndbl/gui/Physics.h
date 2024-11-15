#pragma once

#include "tools/gui/geometry/Space.h"
#include "tools/gui/geometry/SpatialNode2D.h"
#include "ndbl/core/NodeComponent.h"
#include "tools/gui/Size.h"
#include "NodeView.h"
#include "tools/gui/geometry/Pivots.h"

namespace  ndbl
{
    // forward declarations
    class Node;
    class NodeView;
    class ScopeView;

    class Physics : public NodeComponent
    {
    public:
        DECLARE_REFLECT_override

        struct NodeViewConstraint
        {
            typedef std::vector<NodeView*> NodeViews;
            typedef void(NodeViewConstraint::*Rule)(float dt);
            typedef bool(NodeViewConstraint::*Condition)(void) const;

            void          update(float dt);
            void          rule_default(float) {}
            void          rule_1_to_N_as_row(float dt);
            void          rule_N_to_1_as_a_row(float dt);
            void          rule_align_child_scopeviews(float _dt);
            bool          condition_default() const { return true; };

            const char*   name           = "untitled NodeViewConstraint";
            bool          enabled        = true;
            Rule          rule           = &NodeViewConstraint::rule_default;
            Condition     should_apply   = &NodeViewConstraint::condition_default;
            NodeViewFlags leader_flags   = NodeViewFlag_WITH_PINNED;
            NodeViewFlags follower_flags = NodeViewFlag_WITH_PINNED;
            tools::Vec2   leader_pivot   = tools::RIGHT;
            tools::Vec2   follower_pivot = tools::LEFT;
            tools::Vec2   row_direction  = tools::RIGHT;
            tools::Vec2   gap_direction  = tools::CENTER;
            tools::Size   gap_size       = tools::Size_DEFAULT;
            NodeViews     leader;
            NodeViews     follower;

            static std::vector<NodeView*> clean( std::vector<NodeView*>& );
            static bool                   should_follow_output(const Node* node, const Node* output_node );
        };

        typedef std::vector<NodeViewConstraint>  NodeViewConstraints;

        void            init(NodeView*);
        void            add_constraint(NodeViewConstraint& c) { _nodeview_constraints.push_back(std::move(c)); }
        void            apply_constraints(float _dt);
        void            clear_constraints();
        void            add_force( tools::Vec2 force, bool _recurse = false);
        void            add_force_to(tools::Vec2 _target_pos, float _factor, bool _recurse, tools::Space _space );
        void            apply_forces(float _dt);
        bool&           is_active() { return _is_active; };
        NodeViewConstraints&        constraints() { return _nodeview_constraints; };
        const NodeViewConstraints&  constraints() const { return _nodeview_constraints; };

    private:
        bool            _is_active = false;
        NodeView*       _view      = nullptr;
        tools::Vec2     _forces_sum;
        tools::Vec2     _last_frame_forces_sum;
        NodeViewConstraints  _nodeview_constraints;
    };
}

