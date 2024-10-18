#include "Physics.h"

#include <numeric>
#include "tools/core/math.h"
#include "tools/gui/Config.h"
#include "ndbl/core/GraphUtil.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/VariableNode.h"
#include "ndbl/gui/NodeView.h"
#include "Config.h"

using namespace ndbl;
using namespace tools;

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 1
#endif

REFLECT_STATIC_INIT
{
    type::Initializer<Physics>("Physics")
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

void Physics::add_constraint(Physics::Constraint& _constraint)
{
    m_constraints.push_back(std::move(_constraint));
}

void Physics::apply_constraints(float _dt)
{
    if( !is_active ) return;

    for (Physics::Constraint& eachConstraint : m_constraints)
    {
        eachConstraint.update(_dt);
    }
}

void Physics::add_force_to_move_to(tools::Vec2 _target_pos, float _factor, bool _recurse, tools::Space _space)
{
    Vec2 delta   = _target_pos - m_view->xform()->get_pos(_space);
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
    float lensqr_max       = std::pow(100, 4);
    float friction_coef    = lerp(0.0f, 0.5f, m_forces_sum.lensqr() / lensqr_max);
    Vec2  soften_force_sum = Vec2::lerp(m_last_frame_forces_sum, m_forces_sum, 0.95f);
    Vec2  delta            = soften_force_sum * (1.0f - friction_coef) * _dt;

    m_view->xform()->translate( delta );

    m_last_frame_forces_sum = soften_force_sum;
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

        // If current view has a single predecessor, we follow it, except if it is a conditional node
        //
        std::vector<NodeView*> previous_nodes = curr_nodeview->get_adjacent(SlotFlag_PREV);
        if ( !previous_nodes.empty() && !previous_nodes[0]->get_node()->is_conditional() )
        {
            Physics::Constraint constraint("Position below previous", &Physics::Constraint::constrain_1_to_N_as_row);
            constraint.leader         = previous_nodes;
            //constraint.leader_flags   = NodeViewFlag_WITH_RECURSION;
            constraint.follower       = {curr_nodeview};
            constraint.follower_flags = NodeViewFlag_WITH_RECURSION;

            constraint.leader_pivot   = BOTTOM;
            constraint.follower_pivot = TOP;

            if ( constraint.leader.size() == 1)
            {
                constraint.leader_pivot   += LEFT;
                constraint.follower_pivot += LEFT;
            }

            // vertical gap
            constraint.gap_size       = tools::Size_MD;
            constraint.gap_direction  = BOTTOM;

            physics_component->add_constraint(constraint);
        }

        // Align in row Conditional Struct Node's children
        //------------------------------------------------

        std::vector<NodeView*> next = curr_nodeview->get_adjacent(SlotFlag_NEXT);
        if( node->is_conditional() && next.size() >= 1 )
        {
            Physics::Constraint constraint("Align conditional children in a row",
                                           &Physics::Constraint::constrain_N_to_1_as_a_row);
            constraint.leader         = {curr_nodeview};
            constraint.leader_pivot   = BOTTOM_LEFT;
            constraint.leader_flags   = NodeViewFlag_WITH_RECURSION;
            constraint.follower       = next;
            constraint.follower_pivot = TOP_LEFT;
            constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
            constraint.gap_size       = tools::Size_SM;
            constraint.gap_direction  = BOTTOM;
            constraint.row_direction  = RIGHT;
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
            Physics::Constraint constraint("Align many inputs above",
                                           &Physics::Constraint::constrain_N_to_1_as_a_row);

            auto* leader = curr_nodeview;
            constraint.leader         = {leader};
            constraint.leader_pivot   = TOP;
            constraint.follower       = filtered_inputs;
            constraint.follower_pivot = BOTTOM;
            //constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
            constraint.gap_size       = tools::Size_SM;
            constraint.gap_direction  = TOP;

            if ( filtered_inputs.size() >= 2 )
            {
                constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
            }

            if ( leader->get_node()->predecessors().size() + leader->get_node()->successors().size() != 0 )
            {
                constraint.follower_pivot = BOTTOM_LEFT;
                constraint.leader_pivot   = TOP_RIGHT;
                constraint.row_direction  = RIGHT;
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

void Physics::Constraint::update(float _dt)
{
    ASSERT(should_apply != nullptr)
    ASSERT(constrain != nullptr)

    if ( !enabled)
        return;

    if ( !(this->*should_apply)() )
        return;

    (this->*constrain)(_dt);
}

void Physics::Constraint::constrain_1_to_N_as_row(float _dt)
{
    // This type of constrain is designed to make a single NodeView to follow many others

    VERIFY(!leader.empty(), "No leader found!")
    VERIFY(follower.size() == 1, "This is a one to many relationship, a single follower only is allowed")

    std::vector<NodeView*> clean_follower = Physics::Constraint::clean(follower);
    if( clean_follower.empty() )
        return;

    Config* cfg = get_config();
    const NodeView* _follower      = clean_follower[0];
    const BoxShape2D leaders_box          = NodeView::get_rect(leader, WORLD_SPACE, leader_flags);
    const BoxShape2D follower_box         = _follower->get_rect_ex(WORLD_SPACE, follower_flags);

    // Compute how much the follower box needs to be moved to snap the leader's box at a given pivots.
    Vec2 delta = BoxShape2D::diff(leaders_box, leader_pivot , follower_box, follower_pivot );
    delta += gap_direction * cfg->ui_node_gap(gap_size);

    // Apply a force to translate to the (single) follower
    auto* physics_component = _follower->get_node()->get_component<Physics>();
    if( !physics_component )
        return;

    Vec2 current_pos = _follower->xform()->get_pos(WORLD_SPACE);
    Vec2 desired_pos = current_pos + delta;
    physics_component->add_force_to_move_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
}

void Physics::Constraint::constrain_N_to_1_as_a_row(float _dt)
{
    ASSERT(leader.size() == 1)
    ASSERT(follower.size() > 0)

    Config* cfg = get_config();
    std::vector<NodeView*> clean_follower = Physics::Constraint::clean(follower);
    if( clean_follower.empty() )
        return;

    // Form a row with each view box
    std::vector<BoxShape2D>  box(follower.size());
    std::vector<Vec2> delta(follower.size());
    const Vec2        gap = cfg->ui_node_gap(gap_size);

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        box[i] = clean_follower[i]->get_rect_ex(WORLD_SPACE, follower_flags);

        // Determine the delta required to snap the current follower with either the leaders or the previous follower.
        if ( i == 0 )
        {
            // First box is aligned with the leader
            const BoxShape2D leader_box = leader[0]->get_rect_ex(WORLD_SPACE, leader_flags);
            delta[i] = BoxShape2D::diff(leader_box, leader_pivot, box[i], follower_pivot);
            delta[i] += gap * gap_direction;
        }
        else
        {
            // i+1 box is aligned with the i
            delta[i] = BoxShape2D::diff(box[i - 1] , row_direction, box[i], -row_direction);
            delta[i] += gap * row_direction;
            delta[i] -= delta[i-1]; //
        }
    }

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        auto* physics_component = clean_follower[i]->get_node()->get_component<Physics>();
        if( !physics_component )
            continue;
        Vec2 current_pos = clean_follower[i]->xform()->get_pos(WORLD_SPACE);
        Vec2 desired_pos = current_pos + delta[i];
        physics_component->add_force_to_move_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
    }
}

std::vector<NodeView *> Physics::Constraint::clean(std::vector<NodeView *> &views)
{
    std::vector<NodeView *> result;
    for(auto* view : views)
    {
        if (view->visible())
            if (!view->pinned())
                result.push_back(view);
    }
    return result;
}

