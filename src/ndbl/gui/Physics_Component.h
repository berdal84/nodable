#pragma once

#include "tools/core/Component.h"
#include "tools/gui/geometry/Space.h"
#include "ndbl/core/Node.h"
#include "Node_View.h"

namespace  ndbl
{
    // forward declarations
    class Entity;
    class Node;
    class Node_View;
    class Scope_View;

    class Physics_Component : public tools::Component<Node>
    {
    public:
        Physics_Component();
        ~Physics_Component() override = default;

        void  add_force(const tools::Vec2&  force, bool recurse = false);
        void  translate(const tools::Vec2& delta, float speed, bool recursive );
        void  translate_to(const tools::Vec2&  pos, float speed, bool recursive, tools::Space space );
        void  apply_forces(float dt);
        bool& is_active() { return _is_active; };
    private:
        void _on_init();

        bool            _is_active = false;
        Node_View*      _view      = nullptr; 
        tools::Vec2     _forces_sum;
        tools::Vec2     _last_frame_forces_sum;
    };
}

