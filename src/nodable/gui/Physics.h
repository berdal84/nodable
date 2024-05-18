#pragma once
#include "imgui.h"
#include "core/Component.h"
#include "NodeViewConstraint.h"
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
        void            add_force_to_translate_to(fw::vec2 desiredPos, float _factor, bool _recurse = false);
        void            add_force(fw::vec2 force, bool _recurse = false);
        void            apply_forces(float _dt, bool _recurse);
        static void     create_constraints(const std::vector<PoolID<Node>>&);
        static void     destroy_constraints(std::vector<Physics *> &physics_components);
    private:
        PoolID<NodeView> m_view;
        fw::vec2           m_forces_sum;
        fw::vec2           m_last_frame_forces_sum;

        REFLECT_DERIVED_CLASS()
    };
}

