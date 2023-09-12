#include "Physics.h"
#include "NodeView.h"
#include "core/ForLoopNode.h"
#include "core/IConditionalStruct.h"
#include "core/Node.h"
#include "core/NodeUtils.h"
#include "core/algorithm.h"
#include "core/math.h"
#include "core/GraphUtil.h"

using namespace ndbl;
using namespace fw;

REGISTER
{
    fw::registration::push_class<Physics>("Physics")
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

void Physics::add_force_to_translate_to(ImVec2 desiredPos, float _factor, bool _recurse)
{
    ImVec2 delta(desiredPos - m_view->get_position());
    auto factor = std::max(0.0f, _factor);
    add_force(delta * factor, _recurse);
}

void Physics::add_force(ImVec2 force, bool _recurse)
{
    m_forces_sum += force;

    if ( !_recurse ) return;

    for (auto child : m_view->children )
    {
        if (!child->pinned && child->should_follow_output( m_view ))
        {
            if(PoolPoolID<Physics> physics_component = child->get_owner()->get_component<Physics>())
            {
                physics_component->add_force(force, _recurse);
            }
        }
    }
}

void Physics::apply_forces(float _dt, bool _recurse)
{
    float magnitude = std::sqrt(m_forces_sum.x * m_forces_sum.x + m_forces_sum.y * m_forces_sum.y );

    constexpr float magnitude_max  = 1000.0f;
    const float     friction       = fw::math::lerp (0.0f, 0.5f, magnitude / magnitude_max);
    const ImVec2 avg_forces_sum      = (m_forces_sum + m_last_frame_forces_sum) * 0.5f;

    // TODO: handle that outside of this class, or pass view as parameter (begin)
    m_view->translate( avg_forces_sum * ( 1.0f - friction) * _dt , _recurse);
    // TODO: (end)

    m_last_frame_forces_sum = avg_forces_sum;
    m_forces_sum            = ImVec2();
}

void Physics::create_constraints(const std::vector<PoolPoolID<Node>>& nodes)
{
    LOG_VERBOSE("Physics", "create_constraints ...\n");
    for(Node* each_node: Pool::get_pool()->get( nodes ) )
    {
        auto each_view    = each_node->get_component<NodeView>();
        auto each_physics = each_node->get_component<Physics>();
        if ( each_view )
        {
            const fw::type* node_type = each_node->get_type();
            std::vector<PoolPoolID<NodeView>> children_view = GraphUtil::adjacent_components<NodeView>(each_node, Relation::CHILD_PARENT, Way::In); // TODO: cache

            // Follow predecessor Node(s), except if first predecessor is a Conditional if/else
            //---------------------------------------------------------------------------------

            std::vector<PoolPoolID<Node>> predecessor_nodes = each_node->get_predecessors();
            if (!predecessor_nodes.empty() && predecessor_nodes[0]->get_type()->is_not_child_of<IConditionalStruct>() )
            {
                NodeViewConstraint constraint("follow predecessor except if IConditionalStruct", ViewConstraint_t::FollowWithChildren);
                auto predecessor_views = NodeUtils::get_component_ids<NodeView>( predecessor_nodes );
                constraint.add_drivers(predecessor_views);
                constraint.add_target(each_view->poolid());
                each_physics->add_constraint(constraint);

                constraint.apply_when(NodeViewConstraint::always);
            }

            // Align in row Conditional Struct Node's children
            //------------------------------------------------

            if(!children_view.empty() && node_type->is_child_of<IConditionalStruct>() )
            {
                NodeViewConstraint constraint("align IConditionalStruct children", ViewConstraint_t::MakeRowAndAlignOnBBoxBottom);
                constraint.apply_when(NodeViewConstraint::drivers_are_expanded);
                constraint.add_driver(each_view->poolid());
                constraint.add_targets(children_view);

                if (node_type->is<ForLoopNode>() )
                {
                    constraint.add_targets( each_view->successors );
                }
                each_physics->add_constraint(constraint);
            }

            // Align in row Input connected Nodes
            //-----------------------------------

            if ( !each_view->inputs.empty() )
            {
                NodeViewConstraint constraint("align inputs", ViewConstraint_t::MakeRowAndAlignOnBBoxTop);
                constraint.add_driver(each_view->poolid());
                constraint.add_targets( each_view->inputs );
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