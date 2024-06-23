#pragma once

#include "tools/gui/geometry/Space.h"
#include "tools/gui/geometry/XForm2D.h"
#include "ndbl/core/NodeComponent.h"
#include "ndbl/gui/NodeViewConstraint.h"

namespace  ndbl
{
    // forward declarations
    class Node;
    class NodeView;

    class Physics : public NodeComponent {
    public:
        bool            is_active;
        explicit Physics(NodeView*);
        ~Physics() {};
        void            add_constraint(NodeViewConstraint&);
        void            apply_constraints(float _dt);
        void            clear_constraints();
        void            add_force( tools::Vec2 force, bool _recurse = false);
        void            add_force_to_move_to(tools::Vec2 _target_pos, tools::Space _space, float _factor, bool _recurse);
        void            apply_forces(float _dt, bool _recurse);
        std::vector<NodeViewConstraint>& get_constraints() { return m_constraints; };
        const std::vector<NodeViewConstraint>& get_constraints() const { return m_constraints; };
        static void     create_constraints(const std::vector<Node*>&);
        static void     destroy_constraints(std::vector<Physics *> &physics_components);
    private:
        NodeView*   m_view;
        tools::Vec2 m_forces_sum;
        tools::Vec2 m_last_frame_forces_sum;
        std::vector<NodeViewConstraint> m_constraints;
        REFLECT_DERIVED_CLASS()
    };
}

