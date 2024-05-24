#include "Physics.h"

#include "tools/core/math.h"
#include "ndbl/core/ForLoopNode.h"
#include "ndbl/core/GraphUtil.h"
#include "ndbl/core/IConditional.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/NodeUtils.h"

#include "NodeView.h"

using namespace ndbl;
using namespace tools;

REGISTER
{
    registration::push_class<Physics>("Physics")
                     .extends<Component>();
};

Physics::Physics()
    : Component()
    , is_active(true)
{}

Physics::Physics(PoolID<NodeView> view)
    : Physics()
{
    m_view = view;
}

void Physics::clear_constraints()
{
    constraints.clear();
}

void Physics::add_constraint(NodeViewConstraint& _constraint)
{
    constraints.push_back(std::move(_constraint));
}

void Physics::apply_constraints(float _dt)
{
    if( !is_active ) return;

    for (NodeViewConstraint& eachConstraint : constraints)
    {
        eachConstraint.apply(_dt);
    }
}

void Physics::translate_to( tools::Space space, tools::Vec2 target_pos, float _factor, bool _recurse)
{
    Vec2 delta( target_pos - m_view->position(space));
    auto factor = std::max(0.0f, _factor);
    auto force = Vec2::scale(delta, factor);
    add_force( force, _recurse);
}

void Physics::add_force( Vec2 force, bool _recurse)
{
    m_forces_sum += force;

    if ( !_recurse ) return;

    for (PoolID<Node> input_id: m_view->get_owner()->inputs() )
    {
        Node& input = *input_id;
        NodeView& input_view = *input.get_component<NodeView>();
        if ( !input_view.pinned() && input.should_be_constrain_to_follow_output( m_view->get_owner() ))
        {
            if(PoolID<Physics> physics_component = input.get_component<Physics>())
            {
                physics_component->add_force(force, _recurse);
            }
        }
    }
}

void Physics::apply_forces(float _dt, bool _recurse)
{
    float magnitude_max  = 1000.0f;
    float magnitude      = std::sqrt(m_forces_sum.x * m_forces_sum.x + m_forces_sum.y * m_forces_sum.y );
    float friction       = lerp(0.0f, 0.5f, magnitude / magnitude_max);
    Vec2 avg_forces_sum = Vec2::scale(m_forces_sum + m_last_frame_forces_sum, 0.5f);
    Vec2 delta          = Vec2::scale(avg_forces_sum,  (1.0f - friction) * _dt);

    m_view->translate( delta , _recurse);

    m_last_frame_forces_sum = avg_forces_sum;
    m_forces_sum            = Vec2();
}

void Physics::create_constraints(const std::vector<PoolID<Node>>& nodes)
{
    LOG_VERBOSE("Physics", "create_constraints ...\n");
    for(Node* each_node: get_pool()->get( nodes ) )
    {
        auto each_view    = each_node->get_component<NodeView>();
        auto each_physics = each_node->get_component<Physics>();
        if ( each_view )
        {
            const type* node_type = each_node->get_type();

            // Follow predecessor Node(s), except if first predecessor is a Conditional
            //-------------------------------------------------------------------------

            std::vector<PoolID<Node>> predecessor_nodes = each_node->predecessors();
            if (!predecessor_nodes.empty() && predecessor_nodes[0]->get_type()->is_not_child_of<IConditional>() )
            {
                NodeViewConstraint constraint("follow predecessor", ConstrainFlag_LAYOUT_FOLLOW_WITH_CHILDREN );
                auto predecessor_views = NodeUtils::get_component_ids<NodeView>( predecessor_nodes );
                constraint.add_drivers(predecessor_views);
                constraint.add_target(each_view->poolid());
                each_physics->add_constraint(constraint);

                constraint.apply_when(NodeViewConstraint::always);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            std::vector<PoolID<NodeView>> children = each_view->get_adjacent(SlotFlag_CHILD);
            if(!children.empty() && node_type->is_child_of<IConditional>() )
            {
                NodeViewConstraint constraint("align IConditionalStruct children", ConstrainFlag_LAYOUT_MAKE_ROW | ConstrainFlag_ALIGN_BBOX_BOTTOM);
                constraint.apply_when(NodeViewConstraint::drivers_are_expanded);
                constraint.add_driver(each_view->poolid());
                constraint.add_targets( children );
                each_physics->add_constraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------
            std::vector<PoolID<NodeView>> inputs = each_view->get_adjacent(SlotFlag_INPUT);
            if ( !inputs.empty() )
            {
                NodeViewConstraint constraint("align inputs", ConstrainFlag_LAYOUT_MAKE_ROW | ConstrainFlag_ALIGN_BBOX_TOP);
                constraint.add_driver(each_view->poolid());
                constraint.add_targets( inputs );
                each_physics->add_constraint(constraint);
                constraint.apply_when(NodeViewConstraint::always);
            }
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