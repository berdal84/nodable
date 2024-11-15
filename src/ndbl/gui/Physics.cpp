#include "Physics.h"

#include <numeric>
#include "tools/core/math.h"
#include "tools/core/assertions.h"
#include "tools/gui/Config.h"
#include "ndbl/core/Utils.h"
#include "ndbl/core/Node.h"
#include "ndbl/core/VariableNode.h"
#include "ndbl/gui/NodeView.h"
#include "ndbl/gui/ScopeView.h"
#include "Config.h"
#include "tools/gui/geometry/Pivots.h"

using namespace ndbl;
using namespace tools;

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 1
#endif

REFLECT_STATIC_INITIALIZER
(
   DEFINE_REFLECT(Physics).extends<NodeComponent>();
)

void Physics::init(NodeView* view)
{
    ASSERT(view != nullptr);
    _view      = view;
    _is_active = true;
}

void Physics::clear_constraints()
{
    _nodeview_constraints.clear();
    _scopeview_constraints.clear();
}

void Physics::apply_constraints(float _dt)
{
    if( !_is_active )
        return;

    for (auto& constraint : _nodeview_constraints)
        constraint.update(_dt);

    for (auto& constraint : _scopeview_constraints)
        constraint.update(_dt);
}

void Physics::add_force_to(tools::Vec2 _target_pos, float _factor, bool _recurse, tools::Space _space)
{
    Vec2 delta   = _target_pos - _view->spatial_node().position(_space);
    float factor = std::max(0.0f, _factor);
    Vec2 force   = Vec2::scale(delta, factor);
    add_force( force, _recurse);
}

void Physics::add_force( Vec2 force, bool _recurse)
{
    _forces_sum += force;

    if ( !_recurse ) return;

    for (Node* input_node: _view->node()->inputs() )
    {
        NodeView* input_view = input_node->get_component<NodeView>();

        if ( !input_view->pinned())
            if ( NodeViewConstraint::should_follow_output(input_node, _view->node() ))
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

    _view->spatial_node().translate( delta );

    _last_frame_forces_sum = soften_force_sum;
    _forces_sum            = Vec2();
}

void Physics::NodeViewConstraint::update(float _dt)
{
    ASSERT(should_apply != nullptr);
    ASSERT(rule != nullptr);

    if ( !enabled)
        return;

    if ( !(this->*should_apply)() )
        return;

    (this->*rule)(_dt);
}

void Physics::NodeViewConstraint::rule_1_to_N_as_row(float _dt)
{
    // This type of constrain is designed to make a single NodeView to follow many others

    VERIFY(!leader.empty(), "No leader found!");
    VERIFY(follower.size() == 1, "This is a one to many relationship, a single follower only is allowed");

    std::vector<NodeView*> clean_follower = Physics::NodeViewConstraint::clean(follower);
    if( clean_follower.empty() )
        return;

    Config* cfg = get_config();
    const NodeView* _follower      = clean_follower[0];
    const BoxShape2D leaders_box   = NodeView::get_rect(leader, WORLD_SPACE, leader_flags);
    const BoxShape2D follower_box  = _follower->get_rect_ex(WORLD_SPACE, follower_flags);

    // Compute how much the follower box needs to be moved to snap the leader's box at a given pivots.
    Vec2 delta = BoxShape2D::diff(leaders_box, leader_pivot , follower_box, follower_pivot );
    delta += gap_direction * cfg->ui_node_gap(gap_size);

    // Apply a force to translate to the (single) follower
    auto* physics_component = _follower->node()->get_component<Physics>();
    if( !physics_component )
        return;

    Vec2 current_pos = _follower->spatial_node().position(WORLD_SPACE);
    Vec2 desired_pos = current_pos + delta;
    physics_component->add_force_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
}

void Physics::NodeViewConstraint::rule_N_to_1_as_a_row(float _dt)
{
    ASSERT(leader.size() == 1);
    ASSERT(follower.size() > 0);

    Config* cfg = get_config();
    std::vector<NodeView*> clean_follower = Physics::NodeViewConstraint::clean(follower);
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
        Vec2 current_pos = clean_follower[i]->spatial_node().position(WORLD_SPACE);
        Vec2 desired_pos = current_pos + delta[i];
        physics_component->add_force_to(desired_pos, cfg->ui_node_speed, true, WORLD_SPACE);
    }
}

std::vector<NodeView *> Physics::NodeViewConstraint::clean(std::vector<NodeView *> &views)
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

bool Physics::NodeViewConstraint::should_follow_output(const Node* node, const Node* output_node )
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

void Physics::ScopeViewConstraint_ParentChild::update(float dt)
{
    if ( !enabled )
        return;

    // Gather unpinned children
    std::vector<ScopeView*> children;
    for(Scope* child : parent->scope()->child_scope() )
        if ( !child->view()->pinned() )
            children.push_back(child->view() );

    // make a row
    std::vector<Rect> new_rect;
    for(auto child : children)
        new_rect.push_back( child->content_rect() );
    const float gap = get_config()->ui_scope_gap( gap_size );
    Rect::make_row(new_rect, gap );

    // v align
    const Vec2 parent_pivot_pos = parent->node()
                                  ->get_component<NodeView>()
                                  ->shape()
                                  ->pivot( parent_pivot, WORLD_SPACE );
    const float top = parent_pivot_pos.y + gap * gap_direction.y;
    Rect::align_top( new_rect, top );

    // translate views
    for(size_t i = 0; i < children.size(); ++i)
    {
        const Vec2 desired_pos = new_rect[i].center();
        if( auto node = children[i]->scope()->first_node() )
        {
            auto c = node->get_component<Physics>();
            c->add_force_to(desired_pos, get_config()->ui_node_speed, false, WORLD_SPACE);
        }
    }
}
