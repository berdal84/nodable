#pragma once

#include "fw/core/geometry/Space.h"
#include "fw/core/geometry/XForm2D.h"
#include "nodable/core/Component.h"
#include "nodable/gui/NodeViewConstraint.h"

namespace  ndbl
{
    // forward declarations
    class Node;
    class NodeView;
    using fw::PoolID;

    class Physics : public Component {
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
        void            translate_to(fw::Space, fw::Vec2 target_pos, float _factor, bool _recurse = false );
        void            add_force(fw::Vec2 force, bool _recurse = false);
        void            apply_forces(float _dt, bool _recurse);
        static void     create_constraints(const std::vector<PoolID<Node>>&);
        static void     destroy_constraints(std::vector<Physics *> &physics_components);
    private:
        PoolID<NodeView> m_view;
        fw::Vec2 m_forces_sum;
        fw::Vec2 m_last_frame_forces_sum;

        REFLECT_DERIVED_CLASS()
    };
}

