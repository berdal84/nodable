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

    struct ViewConstraint
    {
        typedef std::vector<ASTNodeView*> NodeViews;
        typedef void(ViewConstraint::*Rule)(float dt);

        void          update(float dt);
        void          rule_default(float) {}
        void          rule_1_to_N_as_row(float dt);
        void          rule_N_to_1_as_a_row(float dt);
        void          rule_distribute_sub_scope_views(float _dt);

        const char*   name           = "untitled NodeViewConstraint";
        bool          enabled        = true;
        Rule          rule           = &ViewConstraint::rule_default;
        NodeViewFlags leader_flags   = NodeViewFlag_WITH_PINNED;
        NodeViewFlags follower_flags = NodeViewFlag_WITH_PINNED;
        tools::Vec2   leader_pivot   = tools::RIGHT;
        tools::Vec2   follower_pivot = tools::LEFT;
        tools::Vec2   row_direction  = tools::RIGHT;
        tools::Vec2   gap_direction  = tools::CENTER;
        tools::Size   gap_size       = tools::Size_DEFAULT;
        NodeViews     leader;
        NodeViews     follower;

        static std::vector<ASTNodeView*> clean(std::vector<ASTNodeView*>& );
    };

    class PhysicsComponent : public tools::Component<ASTNode>
    {
    public:
        typedef std::vector<ViewConstraint> Constraints;

        PhysicsComponent();
        ~PhysicsComponent() override = default;

        void            add_constraint(ViewConstraint& c) { _constraints.push_back(std::move(c)); }
        void            apply_constraints(float dt);
        void            clear_constraints();
        void            add_force(const tools::Vec2&  force, bool recurse = false);
        void            translate(const tools::Vec2& delta, float speed, bool recursive );
        void            translate_to(const tools::Vec2&  pos, float speed, bool recursive, tools::Space space );
        void            apply_forces(float dt);
        bool&           is_active() { return _is_active; };
        Constraints&        constraints() { return _constraints; };
        const Constraints&  constraints() const { return _constraints; };

    private:
        void _on_init();

        bool            _is_active = false;
        ASTNodeView*    _view      = nullptr;
        tools::Vec2     _forces_sum;
        tools::Vec2     _last_frame_forces_sum;
        Constraints     _constraints;
    };
}

