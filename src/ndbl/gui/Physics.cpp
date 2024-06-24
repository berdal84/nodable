#include "Physics.h"

#include "tools/core/math.h"
#include "ndbl/core/ForLoopNode.h"
#include "ndbl/core/GraphUtil.h"
#include "ndbl/core/IConditional.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/NodeUtils.h"
#include "ndbl/core/VariableNode.h"

#include "NodeView.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<Physics>("Physics")
                     .extends<NodeComponent>();
};

Physics::Physics(NodeView* view)
: NodeComponent()
, is_active(true)
, m_view(view)
{
    ASSERT(view != nullptr)
}

void Physics::clear_constraints()
{
    m_constraints.clear();
}

void Physics::add_constraint(NodeViewConstraint& _constraint)
{
    m_constraints.push_back(std::move(_constraint));
}

void Physics::apply_constraints(float _dt)
{
    if( !is_active ) return;

    for (NodeViewConstraint& eachConstraint : m_constraints)
    {
        eachConstraint.update(_dt);
    }
}

void Physics::add_force_to_move_to(tools::Vec2 _target_pos, tools::Space _space, float _factor, bool _recurse)
{
    Vec2 delta   = _target_pos - m_view->get_pos(_space);
    float factor = std::max(0.0f, _factor);
    Vec2 force   = Vec2::scale(delta, factor);
    add_force( force, _recurse);
}

void Physics::add_force( Vec2 force, bool _recurse)
{
    m_forces_sum += force;

    if ( !_recurse ) return;

    for (Node* input_id: m_view->get_owner()->inputs() )
    {
        Node& input = *input_id;
        NodeView& input_view = *input.get_component<NodeView>();

        if ( !input_view.pinned())
            if ( input.should_be_constrain_to_follow_output( m_view->get_owner() ))
                if(auto* physics_component = input.get_component<Physics>())
                    physics_component->add_force(force, _recurse);
    }
}

void Physics::apply_forces(float _dt)
{
    float magnitude_max  = 1000.0f;
    float magnitude      = std::sqrt(m_forces_sum.x * m_forces_sum.x + m_forces_sum.y * m_forces_sum.y );
    float friction       = lerp(0.0f, 0.5f, magnitude / magnitude_max);
    Vec2  avg_forces_sum = Vec2::scale(m_forces_sum + m_last_frame_forces_sum, 0.5f);
    Vec2  delta          = Vec2::scale(avg_forces_sum,  (1.0f - friction) * _dt);

    m_view->translate( delta );

    m_last_frame_forces_sum = avg_forces_sum;
    m_forces_sum            = Vec2();
}

void Physics::create_constraints(const std::vector<Node*>& nodes)
{
    LOG_VERBOSE("Physics", "create_constraints ...\n");
    for(Node* node: nodes )
    {
        auto curr_nodeview = node->get_component<NodeView>();
        ASSERT(curr_nodeview != nullptr )

        auto physics_component = node->get_component<Physics>();
        const type* node_type = node->get_class();

        // If current view has a single predecessor, we follow it
        //
        std::vector<Node*> previous_nodes = node->predecessors();
        if ( previous_nodes.size() == 1 )
        {
            NodeViewConstraint constraint("Position below previous", &NodeViewConstraint::constrain_one_to_one);
            constraint.leader         = {previous_nodes[0]->get_component<NodeView>()};
            constraint.follower       = {curr_nodeview};
            constraint.follower_flags = NodeViewFlag_WITH_RECURSION;

            // left edge aligned
            constraint.leader_pivot   = BOTTOM_LEFT;
            constraint.follower_pivot = TOP_LEFT;

            // vertical gap
            constraint.gap_size       = tools::Size_MD;
            constraint.gap_direction  = BOTTOM;

            physics_component->add_constraint(constraint);
        }

        // Align in row Conditional Struct Node's children
        //------------------------------------------------

        std::vector<NodeView*> children = curr_nodeview->get_adjacent(SlotFlag_CHILD);
        if( node_type->is_child_of<IConditional>() && children.size() > 1)
        {
            NodeViewConstraint constraint("Align conditional children in a row", &NodeViewConstraint::constrain_one_to_many_as_a_row);
            constraint.leader         = {curr_nodeview};
            constraint.leader_pivot   = BOTTOM;
            constraint.follower       = children;
            constraint.follower_pivot = TOP;
            constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
            constraint.gap_size       = tools::Size_SM;
            constraint.gap_direction  = RIGHT;
            physics_component->add_constraint(constraint);
        }

        // nodeview's inputs must be aligned on center-top
        // It's a one to many constrain.
        //
        std::vector<NodeView*> inputs = curr_nodeview->get_adjacent(SlotFlag_INPUT);
        std::vector<NodeView*> filtered_inputs;
        for(auto* view : inputs)
        {
            if ( view->get_node()->predecessors().empty() )
                if ( view->get_node()->should_be_constrain_to_follow_output( curr_nodeview->get_node() ) )
                    filtered_inputs.push_back(view);
        }
        if(filtered_inputs.size() > 0 )
        {
            NodeViewConstraint constraint("Align many inputs above", &NodeViewConstraint::constrain_one_to_many_as_a_row);

            constraint.leader         = {curr_nodeview};
            constraint.leader_pivot   = TOP_LEFT;
            constraint.follower       = filtered_inputs;
            constraint.follower_pivot = BOTTOM_LEFT;

            constraint.gap_size       = tools::Size_SM;
            constraint.gap_direction  = TOP;

            constraint.row_direction  = RIGHT;

            if ( constraint.leader[0]->get_node()->is_instruction())
            {
                constraint.follower_pivot = BOTTOM_LEFT;
                constraint.leader_pivot   = TOP_RIGHT;
            }

            physics_component->add_constraint(constraint);
        }
    }
    LOG_VERBOSE("Physics", "create_constraints OK\n");
}

void Physics::destroy_constraints(std::vector<Physics*> &physics_components)
{
    LOG_VERBOSE("Physics", "destroy_constraints ...\n");
    for(Physics* physics: physics_components)
    {
        physics->clear_constraints();
    }
    LOG_VERBOSE("Physics", "destroy_constraints OK\n");
}