#pragma once

#include <vector>
#include "imgui.h"
#include "core/reflection/reflection"

namespace ndbl {

    // forward declarations
    class NodeView;

    enum class ViewConstraint_t {
        AlignOnBBoxLeft,
        MakeRowAndAlignOnBBoxTop,
        MakeRowAndAlignOnBBoxBottom,
        FollowWithChildren,
    };

    R_ENUM(ViewConstraint_t)
    R_ENUM_VALUE(AlignOnBBoxLeft)
    R_ENUM_VALUE(MakeRowAndAlignOnBBoxTop)
    R_ENUM_VALUE(MakeRowAndAlignOnBBoxBottom)
    R_ENUM_VALUE(FollowWithChildren)
    R_ENUM_END

    /**
     * A class to abstract a constraint between some NodeView
     */
    class NodeViewConstraint {
    public:
        using Filter = std::function<bool(NodeViewConstraint*)>;

        NodeViewConstraint(const char* _name, ViewConstraint_t _type);
        void apply(float _dt);
        void apply_when(const Filter& _lambda) { m_filter = _lambda; }
        void add_target(NodeView*);
        void add_driver(NodeView*);
        void add_targets(const std::vector<NodeView*>&);
        void add_drivers(const std::vector<NodeView*>&);
        void draw_view();

        ImVec2 m_offset; // offset applied to the constrain

        static const Filter no_target_expanded;
        static const Filter drivers_are_expanded;
        static const Filter always;

    private:
        const char*       m_name;
        bool              m_is_active;
        Filter            m_filter; // Lambda returning true if this constrain should apply.
        ViewConstraint_t  m_type;
        std::vector<NodeView*> m_drivers; // driving the targets
        std::vector<NodeView*> m_targets;
    };
}