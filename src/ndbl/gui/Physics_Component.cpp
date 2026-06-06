#include "Physics_Component.h"

#include <numeric>
#include <ranges> // for std::iota
#include "tools/core/Math.h"
#include "tools/core/Asserts.h"
#include "tools/core/Component.h"
#include "tools/gui/Config.h"
#include "ndbl/core/Node.h"
#include "ndbl/gui/Node_View.h"
#include "ndbl/gui/Scope_View.h"
#include "Config.h"
#include "tools/gui/geometry/Pivots.h"

using namespace ndbl;
using namespace tools;

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 1
#endif

Physics_Component::Physics_Component()
: Component<Node>("Physics")
{
    Component::signal_init.connect<&Physics_Component::_on_init>(this);
}

void Physics_Component::_on_init()
{
    _view      = entity()->component<Node_View>();
    ASSERT(_view);
    _is_active = true;
}

void Physics_Component::translate(const tools::Vec2& delta, float speed, bool recursive)
{
    const Vec2 force = delta * speed;
    add_force(force, recursive);
}

void Physics_Component::translate_to(const tools::Vec2& pos, float speed, bool recursive, tools::Space space)
{
    const Vec2 delta = pos - _view->spatial_node()->position(space);
    const Vec2 force = delta * speed;
    add_force(force, recursive);
}

void Physics_Component::add_force(const tools::Vec2& force, bool _recurse)
{
    _forces_sum += force;

    if ( !_recurse ) return;

    for (Node* input_node: _view->node()->inputs() )
    {
        Node_View* input_view = input_node->component<Node_View>();

        if ( !input_view->state()->pinned())
            if (node_is_output_node_in_expression(input_node, _view->node()))
                if(auto* physics_component = input_node->component<Physics_Component>())
                    physics_component->add_force(force, _recurse);
    }
}

void ndbl::Physics_Component::apply_forces(float _dt)
{
    float lensqr_max       = std::pow(100, 4);
    float friction_coef    = tools::clamped_lerp(0.0f, 0.5f, _forces_sum.lensqr() / lensqr_max);
    Vec2  soften_force_sum = Vec2::lerp(_last_frame_forces_sum, _forces_sum, 0.95f);
    Vec2  delta            = soften_force_sum * (1.0f - friction_coef) * _dt;

    _view->spatial_node()->translate( delta );

    _last_frame_forces_sum = soften_force_sum;
    _forces_sum            = Vec2();
}
