#include "Physics.h"

#include <numeric>
#include "tools/core/math.h"
#include "tools/gui/Config.h"
#include "ndbl/core/Utils.h"
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

void Physics::init(NodeView* view)
{
    ASSERT(view != nullptr);
    _view      = view;
    _is_active = true;
}

void Physics::clear_constraints()
{
    _constraints.clear();
}

void Physics::add_constraint(Physics::Constraint& _constraint)
{
    _constraints.push_back(std::move(_constraint));
}

void Physics::apply_constraints(float _dt)
{
    if( !_is_active ) return;

    for (Physics::Constraint& eachConstraint : _constraints)
    {
        eachConstraint.update(_dt);
    }
}

void Physics::add_force_to_move_to(tools::Vec2 _target_pos, float _factor, bool _recurse, tools::Space _space)
{
    Vec2 delta   = _target_pos - _view->xform()->position(_space);
    float factor = std::max(0.0f, _factor);
    Vec2 force   = Vec2::scale(delta, factor);
    add_force( force, _recurse);
}

void Physics::add_force( Vec2 force, bool _recurse)
{
    _forces_sum += force;

    if ( !_recurse ) return;

    for (Node* input_node: _view->get_owner()->inputs() )
    {
        NodeView* input_view = input_node->get_component<NodeView>();

        if ( !input_view->pinned())
            if ( Constraint::should_follow_output(input_node, _view->get_owner() ))
                if(auto* physics_component = input_node->get_component<Physics>())
                    physics_component->add_force(force, _recurse);
    }
}

void Physics::apply_forces(float _dt)
{
    float lensqr_max       = std::pow(100, 4);
    float friction_coef    = lerp(0.0f, 0.5f, _forces_sum.lensqr() / lensqr_max);
    Vec2  soften_force_sum = Vec2::lerp(_last_frame_forces_sum, _forces_sum, 0.95f);
    Vec2  delta            = soften_force_sum * (1.0f - friction_coef) * _dt;

    _view->xform()->translate(delta );

    _last_frame_forces_sum = soften_force_sum;
    _forces_sum            = Vec2();
}

void Physics::update(float dt, const std::vector<Physics *>& components, bool dirty)
{
    LOG_VERBOSE("Physics", "Updating constraints ...\n");
    if ( dirty )
    {
        LOG_VERBOSE("Physics", "Constraints are dirty, refreshing ...\n");
        for(Physics* c : components)
            c->clear_constraints();
        Physics::_create_constraints(components);
    }

    for (auto c : components)
        c->apply_constraints(dt);

    for(auto c : components)
        c->apply_forces(dt);

    LOG_VERBOSE("Physics", "Constraints updated.\n");
}

void Physics::_create_constraints(const std::vector<Physics*>& physics)
{
    LOG_VERBOSE("Physics", "_create_constraints ...\n");
    for(Physics* physic : physics )
    {
        Node* node = physic->get_owner();
        auto curr_nodeview = node->get_component<NodeView>();
        ASSERT(curr_nodeview != nullptr );

        auto physics_component = node->get_component<Physics>();

        // If current view has a single predecessor, we follow it, except if it is a conditional node
        //
        std::vector<NodeView*> previous_nodes = curr_nodeview->get_adjacent(SlotFlag_FLOW_IN);
        if ( !previous_nodes.empty() && !Utils::is_conditional(previous_nodes[0]->node() ) )
        {
            Constraint constraint("Position below previous", &Constraint::constrain_1_to_N_as_row);
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

        std::vector<NodeView*> next = curr_nodeview->get_adjacent(SlotFlag_FLOW_OUT);
        if( Utils::is_conditional( node ) && next.size() >= 1 )
        {
            Constraint constraint("Align conditional child_node in a row", &Constraint::constrain_N_to_1_as_a_row);
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
            Node* _node = view->node();
            if (_node->flow_inputs().empty() )
                if ( Constraint::should_follow_output( _node, curr_nodeview->node() ) )
                    filtered_inputs.push_back(view);
        }
        if(filtered_inputs.size() > 0 )
        {
            Constraint constraint("Align many inputs above", &Constraint::constrain_N_to_1_as_a_row);

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

            if (leader->node()->flow_inputs().size() + leader->node()->flow_outputs().size() != 0 )
            {
                constraint.follower_pivot = BOTTOM_LEFT;
                constraint.leader_pivot   = TOP_RIGHT;
                constraint.row_direction  = RIGHT;
            }

            physics_component->add_constraint(constraint);
        }
    }
    LOG_VERBOSE("Physics", "_create_constraints OK\n");
}

void Physics::Constraint::update(float _dt)
{
    ASSERT(should_apply != nullptr);
    ASSERT(constrain != nullptr);

    if ( !enabled)
        return;

    if ( !(this->*should_apply)() )
        return;

    (this->*constrain)(_dt);
}

void Physics::Constraint::constrain_1_to_N_as_row(float _dt)
{
    // This type of constrain is designed to make a single NodeView to follow many others

    VERIFY(!leader.empty(), "No leader found!");
    VERIFY(follower.size() == 1, "This is a one to many relationship, a single follower only is allowed");

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
    auto* physics_component = _follower->node()->get_component<Physics>();
    if( !physics_component )
        return;

    Vec2 current_pos = _follower->xform()->position(WORLD_SPACE);
    Vec2 desired_pos = current_pos + delta;
    physics_component->add_force_to_move_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
}

void Physics::Constraint::constrain_N_to_1_as_a_row(float _dt)
{
    ASSERT(leader.size() == 1);
    ASSERT(follower.size() > 0);

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
        auto* physics_component = clean_follower[i]->node()->get_component<Physics>();
        if( !physics_component )
            continue;
        Vec2 current_pos = clean_follower[i]->xform()->position(WORLD_SPACE);
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

bool Physics::Constraint::should_follow_output(const Node* node, const Node* output_node )
{
    ASSERT(node != nullptr);
    ASSERT(output_node != nullptr);

    // Instruction should never follow an output (they must stick to the codeflow)
    if ( !Utils::is_instruction( node ) )
    {
        VERIFY( !node->outputs().empty(), "You should call this method knowing that other is in child_node's outputs, which means the vector is not empty.");
        const bool is_first_element = node->outputs().front() == output_node;
        return is_first_element;
    }

    // However, variables can be declared inlined (like in an if condition:  "if (int i = 0) {...}" )
    // In that case we want the variable to follow the output.
    if( node->type() == NodeType_VARIABLE )
    {
        auto variable = static_cast<const VariableNode*>( node );
        if ( auto adjacent = variable->decl_out()->first_adjacent() )
            return adjacent->node == output_node;

    }

    return false;
}
