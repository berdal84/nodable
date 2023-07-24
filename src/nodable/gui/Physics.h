#pragma once
#include "imgui.h"
#include "core/Component.h"
#include "NodeViewConstraint.h"
namespace  ndbl
{
    // forward declarations
    class Node;
    class NodeView;

    class Physics : public Component {
    public:
        bool            is_active;
        std::vector<NodeViewConstraint> constraints;

        Physics(NodeView* _view);
        void            add_constraint(NodeViewConstraint&);
        void            apply_constraints(float _dt);
        void            clear_constraints();
        void            add_force_to_translate_to(ImVec2 desiredPos, float _factor, bool _recurse = false);
        void            add_force(ImVec2 force, bool _recurse = false);
        void            apply_forces(float _dt, bool _recurse);
        static void     create_constraints(const std::vector<Node*>& nodes);
        static void     destroy_constraints(const std::vector<Physics*>& nodes);
    private:
        NodeView*       m_view;
        bool            m_edition_enable;
        ImVec2          m_forces_sum;
        ImVec2          m_last_frame_forces_sum;

        REFLECT_DERIVED_CLASS()
    };
}

