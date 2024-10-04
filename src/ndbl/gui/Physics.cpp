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
#define DEBUG_DRAW 0
#endif

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
    float lensqr_max       = std::pow(100, 4);
    float friction_coef    = lerp(0.0f, 0.5f, m_forces_sum.lensqr() / lensqr_max);
    Vec2  soften_force_sum = Vec2::lerp(m_last_frame_forces_sum, m_forces_sum, 0.95f);

    m_view->translate(soften_force_sum * (1.0f - friction_coef) * _dt );

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
        if ( previous_nodes.size() == 1 && !previous_nodes[0]->get_node()->is_conditional() )
        {
            Physics::Constraint constraint("Position below previous", &Physics::Constraint::constrain_one_to_one);
            constraint.leader         = {previous_nodes[0]};
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

        std::vector<NodeView*> next = curr_nodeview->get_adjacent(SlotFlag_NEXT);
        if( node->is_conditional() && next.size() > 1 )
        {
            Physics::Constraint constraint("Align conditional children in a row", &Physics::Constraint::constrain_one_to_many_as_a_row);
            constraint.leader         = {curr_nodeview};
            constraint.leader_pivot   = BOTTOM_LEFT;
            //constraint.leader_flags   = NodeViewFlag_WITH_RECURSION;
            constraint.follower       = next;
            constraint.follower_pivot = TOP_LEFT;
            constraint.follower_flags = NodeViewFlag_WITH_RECURSION;
            constraint.gap_size       = tools::Size_SM;
            constraint.gap_direction  = BOTTOM;
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
            Physics::Constraint constraint("Align many inputs above", &Physics::Constraint::constrain_one_to_many_as_a_row);

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

void Physics::Constraint::constrain_one_to_one(float _dt)
{
    ASSERT(leader.size() == 1)
    ASSERT(follower.size() == 1)

    std::vector<NodeView*> clean_follower = Physics::Constraint::clean(follower);
    if( clean_follower.empty() )
        return;

    const Box leader_box   = leader[0]->get_rect_ex(SCREEN_SPACE, leader_flags);
    const Box old_follower_box_noflags = clean_follower[0]->get_rect_ex(SCREEN_SPACE, SlotFlag_NONE);
    const Box old_follower_box = clean_follower[0]->get_rect_ex(SCREEN_SPACE, follower_flags);

    // Move the current node box close to the previous, by following the align_item vector.

    Box new_follower_box = Box::align(leader_box, leader_pivot , old_follower_box_noflags, follower_pivot );

    Vec2 gap = gap_direction * get_config()->ui_node_gap(gap_size);
    new_follower_box.translate(gap);

    // follower bbox with no flags and flags may differ, we need to apply an offset in that case.
    Vec2 offset = Vec2::distance(old_follower_box.get_pivot(follower_pivot),old_follower_box_noflags.get_pivot(follower_pivot));
    new_follower_box.translate(offset*gap_direction);

    // Use the Physics component to apply a force to translate to the box
    auto* physics_component = follower[0]->get_node()->get_component<Physics>();
    Config* cfg = get_config();
    physics_component->add_force_to_move_to(new_follower_box.get_pos(), SCREEN_SPACE, cfg->ui_node_speed, true);
}

void Physics::Constraint::constrain_one_to_many_as_a_row(float _dt)
{
    ASSERT(leader.size() == 1)
    ASSERT(follower.size() > 0)

    Config* cfg = get_config();
    std::vector<NodeView*> clean_follower = Physics::Constraint::clean(follower);
    if( clean_follower.empty() )
        return;

    // Form a row with each view box
    std::vector<Box> old_box;
    std::vector<Box> new_box;
    const Vec2 gap = get_config()->ui_node_gap(gap_size);

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        Box box         = clean_follower[i]->get_rect_ex(SCREEN_SPACE, follower_flags);
        Box box_noflags = clean_follower[i]->get_rect_ex(SCREEN_SPACE, NodeViewFlag_NONE);
        old_box.push_back(box);

        if ( i == 0 )
        {
            // First box is aligned with the leader
            const Box leader_box = leader[0]->get_rect_ex(SCREEN_SPACE, leader_flags);
            box = Box::align(leader_box, leader_pivot, box, follower_pivot);
            box.translate(gap * gap_direction);
            box.translate(box.get_pivot(follower_pivot) - box_noflags.get_pivot(follower_pivot));
        }
        else
        {
            // i+1 box is aligned with the i
            box = Box::align(new_box.back(), row_direction, box, -row_direction);
            // There is a gap between each box
            box.translate( gap * row_direction );
        }

        new_box.emplace_back(box);
    }

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        auto* physics_component = clean_follower[i]->get_node()->get_component<Physics>();
        ImGuiEx::DebugLine(old_box[i].get_pos(), new_box[i].get_pos(), ImColor(0,255,0), 4.f); // green line to symbolize the desired path
        physics_component->add_force_to_move_to(new_box[i].get_pos(), SCREEN_SPACE, cfg->ui_node_speed, true);
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

