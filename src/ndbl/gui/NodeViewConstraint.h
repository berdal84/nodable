#pragma once

#include "tools/gui/geometry/Vec2.h"
#include "tools/core/memory/memory.h"
#include "NodeView.h"
#include "tools/gui/Size.h"
#include <vector>
#include <functional>

namespace ndbl
{
    // forward declarations
    class NodeView;

    class NodeViewConstraint
    {
    public:
        typedef void(NodeViewConstraint::*Constrain)(float _dt);
        typedef bool(NodeViewConstraint::*Rule)(void);

        static const Rule no_target_expanded;
        static const Rule drivers_are_expanded;

        NodeViewConstraint(
            const char *_name,
            Constrain constrain,
            Rule rule = &NodeViewConstraint::rule_always
        )
        : name(_name)
        , constrain(constrain)
        {}

        void update(float _dt);

        void constrain_one_to_one(float _dt);
        void constrain_one_to_many_as_a_row(float _dt);
        void constrain_many_to_one(float _dt);

        bool rule_always() { return true; };
        bool rule_no_target_expanded();
        bool rule_drivers_are_expanded();

        static std::vector<NodeView*> clean(std::vector<NodeView*> &views);
        void draw_ui();

        const char*   name;

        Constrain     constrain      = nullptr ;
        bool          enabled        = true;
        Rule          should_apply   = &NodeViewConstraint::rule_always;
        NodeViewFlags leader_flags   = NodeViewFlag_WITH_PINNED;
        NodeViewFlags follower_flags = NodeViewFlag_WITH_PINNED;
        bool          recurse_followers = false;
        tools::Vec2   leader_pivot   =  tools::RIGHT;
        tools::Vec2   follower_pivot =  tools::LEFT;
        tools::Vec2   row_direction  =  tools::RIGHT;
        tools::Vec2   gap_direction  =  tools::CENTER;
        tools::Size   gap_size       = tools::Size_DEFAULT;
        std::vector<NodeView*> leader;
        std::vector<NodeView*> follower;
    };
}
