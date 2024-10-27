#pragma once

#include "tools/gui/geometry/Space.h"
#include "tools/gui/geometry/SpatialNode2D.h"
#include "ndbl/core/NodeComponent.h"
#include "tools/gui/Size.h"
#include "NodeView.h"

namespace  ndbl
{
    // forward declarations
    class Node;
    class NodeView;

    class Physics : public NodeComponent
    {
    public:

        struct Constraint
        {
        public:
            typedef void(Constraint::*Constrain)(float _dt);
            typedef bool(Constraint::*Rule)(void);

            Constraint(
                const char* name,
                Constrain   constrain,
                Rule        rule = &Constraint::rule_always
            )
            : name(name)
            , constrain(constrain)
            , should_apply(rule)
            {}

            void update(float _dt);

            void constrain_1_to_N_as_row(float _dt);
            void constrain_N_to_1_as_a_row(float _dt);
            //void constrain_many_to_one(float _dt);

            bool rule_always() { return true; };
            //bool rule_no_target_expanded();
            //bool rule_drivers_are_expanded();

            const char*   name;
            Constrain     constrain      = nullptr ;
            bool          enabled        = true;
            Rule          should_apply   = &Constraint::rule_always;
            NodeViewFlags leader_flags   = NodeViewFlag_WITH_PINNED;
            NodeViewFlags follower_flags = NodeViewFlag_WITH_PINNED;
            tools::Vec2   leader_pivot   =  tools::RIGHT;
            tools::Vec2   follower_pivot =  tools::LEFT;
            tools::Vec2   row_direction  =  tools::RIGHT;
            tools::Vec2   gap_direction  =  tools::CENTER;
            tools::Size   gap_size       = tools::Size_DEFAULT;
            std::vector<NodeView*> leader;
            std::vector<NodeView*> follower;

            static std::vector<NodeView*> clean( std::vector<NodeView*>& );
            static bool should_follow_output(const Node* node, const Node* output_node );
        };

        bool            is_active;
        explicit Physics(NodeView*);
        ~Physics() {};
        void            add_constraint(Constraint&);
        void            apply_constraints(float _dt);
        void            clear_constraints();
        void            add_force( tools::Vec2 force, bool _recurse = false);
        void            add_force_to_move_to(tools::Vec2 _target_pos, float _factor, bool _recurse, tools::Space _space );
        void            apply_forces(float _dt);
        std::vector<Constraint>& get_constraints() { return m_constraints; };
        const std::vector<Constraint>& get_constraints() const { return m_constraints; };
        static void     update(float dt, const std::vector<Physics *>&, bool dirty);

    private:
        static void     _create_constraints(const std::vector<Physics*>&);

        NodeView*   m_view;
        tools::Vec2 m_forces_sum;
        tools::Vec2 m_last_frame_forces_sum;
        std::vector<Constraint> m_constraints;
        REFLECT_DERIVED_CLASS()
    };
}

