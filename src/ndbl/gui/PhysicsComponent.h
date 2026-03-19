#pragma once

#include "tools/core/Component.h"
#include "tools/gui/geometry/Space.h"
#include "tools/gui/geometry/SpatialNode.h"
#include "tools/gui/Size.h"
#include "tools/gui/geometry/Pivots.h"
#include "ASTNodeView.h"
#include "ndbl/core/ASTNode.h"

namespace  ndbl
{
    // forward declarations
    class Entity;
    class ASTNode;
    class ASTNodeView;
    class ASTScopeView;

    class PhysicsComponent : public tools::Component<ASTNode>
    {
    public:
        PhysicsComponent();
        ~PhysicsComponent() override = default;

        void  add_force(const tools::Vec2&  force, bool recurse = false);
        void  translate(const tools::Vec2& delta, float speed, bool recursive );
        void  translate_to(const tools::Vec2&  pos, float speed, bool recursive, tools::Space space );
        void  apply_forces(float dt);
        bool& is_active() { return _is_active; };
    private:
        void _on_init();

        bool            _is_active = false;
        ASTNodeView*    _view      = nullptr; 
        tools::Vec2     _forces_sum;
        tools::Vec2     _last_frame_forces_sum;
    };
}

