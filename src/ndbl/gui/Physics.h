#pragma once

#include "tools/core/geometry/Space.h"
#include "tools/core/geometry/XForm2D.h"
#include "ndbl/core/NodeComponent.h"
#include "ndbl/gui/NodeViewConstraint.h"

namespace  ndbl
{
    // forward declarations
    class Node;
    class NodeView;
    using tools::PoolID;

    class Physics : public NodeComponent {
    public:
        bool            is_active;
        std::vector<NodeViewConstraint> constraints;
        Physics();
        Physics(PoolID<NodeView> view);
        ~Physics() = default;
        Physics(Physics&&) = default;
        Physics& operator=(Physics&&) = default;
        void            add_constraint(NodeViewConstraint&);
        void            apply_constraints(float _dt);
        void            clear_constraints();
        void            translate_to( tools::Space, tools::Vec2 target_pos, float _factor, bool _recurse = false );
        void            add_force( tools::Vec2 force, bool _recurse = false);
        void            apply_forces(float _dt, bool _recurse);
        static void     create_constraints(const std::vector<PoolID<Node>>&);
        static void     destroy_constraints(std::vector<Physics *> &physics_components);
    private:
        PoolID<NodeView> m_view;
        tools::Vec2 m_forces_sum;
        tools::Vec2 m_last_frame_forces_sum;

        REFLECT_DERIVED_CLASS()
    };
}

