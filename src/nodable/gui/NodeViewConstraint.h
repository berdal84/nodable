#pragma once

#include <vector>
#include "imgui.h"
#include "core/reflection/reflection"
#include "core/Pool.h"

namespace ndbl {

    // forward declarations
    class NodeView;
    using fw::PoolID;

    typedef int ConstrainFlags;
    enum ConstrainFlag_
    {
        ConstrainFlag_LAYOUT_DEFAULT              = 1 << 0,
        ConstrainFlag_LAYOUT_MAKE_ROW             = 1 << 1,
        ConstrainFlag_ALIGN_BBOX_LEFT             = 1 << 2,
        ConstrainFlag_ALIGN_BBOX_TOP              = 1 << 3,
        ConstrainFlag_ALIGN_BBOX_BOTTOM           = 1 << 4,
        ConstrainFlag_LAYOUT_FOLLOW_WITH_CHILDREN = ConstrainFlag_LAYOUT_DEFAULT | ConstrainFlag_ALIGN_BBOX_BOTTOM,
        ConstrainFlag_ALIGN_MASK                  = ConstrainFlag_ALIGN_BBOX_LEFT
                                                  | ConstrainFlag_ALIGN_BBOX_TOP
                                                  | ConstrainFlag_ALIGN_BBOX_BOTTOM,
        ConstrainFlag_LAYOUT_MASK                 = ConstrainFlag_LAYOUT_MAKE_ROW | ConstrainFlag_LAYOUT_DEFAULT,
    };

    R_ENUM( ConstrainFlag_ )
    R_ENUM_VALUE( ConstrainFlag_LAYOUT_MAKE_ROW )
    R_ENUM_VALUE(ConstrainFlag_LAYOUT_FOLLOW_WITH_CHILDREN)
    R_ENUM_VALUE(ConstrainFlag_ALIGN_BBOX_LEFT)
    R_ENUM_VALUE(ConstrainFlag_ALIGN_BBOX_TOP)
    R_ENUM_VALUE(ConstrainFlag_ALIGN_BBOX_BOTTOM)
    R_ENUM_VALUE(ConstrainFlag_ALIGN_MASK)
    R_ENUM_VALUE(ConstrainFlag_LAYOUT_MASK)
    R_ENUM_END

    /**
     * A class to abstract a constraint between some NodeView
     */
    class NodeViewConstraint {
    public:
        using Filter = std::function<bool(NodeViewConstraint*)>;

        NodeViewConstraint(const char* _name, ConstrainFlags );
        void apply(float _dt);
        void apply_when(const Filter& _lambda) { m_filter = _lambda; }
        void add_target(PoolID<NodeView>);
        void add_driver(PoolID<NodeView>);
        void add_targets(const std::vector<PoolID<NodeView>>&);
        void add_drivers(const std::vector<PoolID<NodeView>>&);
        void draw_view();

        ImVec2 m_offset; // offset applied to the constraint

        static const Filter no_target_expanded;
        static const Filter drivers_are_expanded;
        static const Filter always;

    private:
        const char*       m_name;
        bool              m_is_active;
        Filter            m_filter; // Lambda returning true if this constrain should apply.
        ConstrainFlags    m_flags;
        std::vector<PoolID<NodeView>> m_drivers; // driving the targets
        std::vector<PoolID<NodeView>> m_targets;
    };
}