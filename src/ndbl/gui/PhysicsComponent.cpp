#include "PhysicsComponent.h"

#include <numeric>
#include <ranges> // for std::iota
#include "tools/core/math.h"
#include "tools/core/assertions.h"
#include "tools/core/Component.h"
#include "tools/gui/Config.h"
#include "ndbl/core/ASTUtils.h"
#include "ndbl/core/ASTNode.h"
#include "ndbl/gui/ASTNodeView.h"
#include "ndbl/gui/ASTScopeView.h"
#include "Config.h"
#include "tools/gui/geometry/Pivots.h"

using namespace ndbl;
using namespace tools;

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 1
#endif

PhysicsComponent::PhysicsComponent()
: Component<ASTNode>("Physics")
{
    Component::signal_init.connect<&PhysicsComponent::_on_init>(this);
}

void PhysicsComponent::_on_init()
{
    _view      = entity()->component<ASTNodeView>();
    ASSERT(_view);
    _is_active = true;
}

void PhysicsComponent::translate(const tools::Vec2& delta, float speed, bool recursive)
{
    const Vec2 force = delta * speed;
    add_force(force, recursive);
}

void PhysicsComponent::translate_to(const tools::Vec2& pos, float speed, bool recursive, tools::Space space)
{
    const Vec2 delta = pos - _view->spatial_node()->position(space);
    const Vec2 force = delta * speed;
    add_force(force, recursive);
}

void PhysicsComponent::add_force(const tools::Vec2& force, bool _recurse)
{
    _forces_sum += force;

    if ( !_recurse ) return;

    for (ASTNode* input_node: _view->node()->inputs() )
    {
        ASTNodeView* input_view = input_node->component<ASTNodeView>();

        if ( !input_view->state()->pinned())
            if (ASTUtils::is_output_node_in_expression(input_node, _view->node()))
                if(auto* physics_component = input_node->component<PhysicsComponent>())
                    physics_component->add_force(force, _recurse);
    }
}

void ndbl::PhysicsComponent::apply_forces(float _dt)
{
    float lensqr_max       = std::pow(100, 4);
    float friction_coef    = tools::clamped_lerp(0.0f, 0.5f, _forces_sum.lensqr() / lensqr_max);
    Vec2  soften_force_sum = Vec2::lerp(_last_frame_forces_sum, _forces_sum, 0.95f);
    Vec2  delta            = soften_force_sum * (1.0f - friction_coef) * _dt;

    _view->spatial_node()->translate( delta );

    _last_frame_forces_sum = soften_force_sum;
    _forces_sum            = Vec2();
}
