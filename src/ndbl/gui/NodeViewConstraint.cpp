#include "NodeViewConstraint.h"
#include "Config.h"
#include "ndbl/core/ForLoopNode.h"
#include "ndbl/gui/NodeView.h"
#include "ndbl/gui/Physics.h"
#include "tools/gui/Config.h"
#include "tools/core/math.h"
#include <numeric>

using namespace ndbl;
using namespace tools;

#ifdef NDBL_DEBUG
#define DEBUG_DRAW 0
#endif

void NodeViewConstraint::update(float _dt)
{
    ASSERT(should_apply != nullptr)
    ASSERT(constrain != nullptr)

    if ( !enabled)
        return;

    if ( !(this->*should_apply)() )
        return;

    (this->*constrain)(_dt);
}

void NodeViewConstraint::constrain_one_to_one(float _dt)
{
    ASSERT(leader.size() == 1)
    ASSERT(follower.size() == 1)

    std::vector<NodeView*> clean_follower = NodeViewConstraint::clean(follower);
    if( clean_follower.empty() )
        return;

    const Box leader_box   = leader[0]->get_rect_ex(SCREEN_SPACE, leader_flags);
    const Box old_follower_box_noflags = clean_follower[0]->get_rect_ex(SCREEN_SPACE, SlotFlag_NONE);
    const Box old_follower_box = clean_follower[0]->get_rect_ex(SCREEN_SPACE, follower_flags);

    // Move the current node box close to the previous, by following the align_item vector.

    Box new_follower_box = Box::align(leader_box, leader_pivot , old_follower_box_noflags, follower_pivot );

    Vec2 gap = gap_direction * get_config()->ui_node_gap(gap_size);
    new_follower_box.translate(gap);

    // follower bbox with no flags and flags may differ, we need to apply an offset in that case.
    Vec2 offset = Vec2::distance(old_follower_box.get_pivot(follower_pivot),old_follower_box_noflags.get_pivot(follower_pivot));
    new_follower_box.translate(offset*gap_direction);

    // Use the Physics component to apply a force to translate to the box
    auto* physics_component = follower[0]->get_node()->get_component<Physics>();
    Config* cfg = get_config();
    physics_component->add_force_to_move_to(new_follower_box.get_pos(), SCREEN_SPACE, cfg->ui_node_speed, true);
}

void NodeViewConstraint::constrain_one_to_many_as_a_row(float _dt)
{
    ASSERT(leader.size() == 1)
    ASSERT(follower.size() > 0)

    Config* cfg = get_config();
    std::vector<NodeView*> clean_follower = NodeViewConstraint::clean(follower);
    if( clean_follower.empty() )
        return;

    // Form a row with each view box
    std::vector<Box> old_box;
    std::vector<Box> new_box;
    Vec2 gap_items = row_direction * get_config()->ui_node_gap(gap_size);
    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        Box box = clean_follower[i]->get_rect_ex(SCREEN_SPACE, follower_flags);
        old_box.push_back(box);

        bool is_first = i == 0;
        if ( is_first )
        {
            // First box is aligned with the leader
            const Box leader_box = leader[0]->get_rect_ex(SCREEN_SPACE, leader_flags);
            box = Box::align(leader_box, leader_pivot, box, follower_pivot);
            Vec2 gap = gap_direction * get_config()->ui_node_gap(gap_size);
            box.translate(gap);
        }
        else
        {
            // i+1 box is aligned with the i
            box = Box::align(new_box.back(), row_direction, box, -row_direction);
            // There is a gap between each box
            box.translate(gap_items);
        }

        new_box.emplace_back(box);
    }

    for(size_t i = 0; i < clean_follower.size(); i++)
    {
        auto* physics_component = clean_follower[i]->get_node()->get_component<Physics>();
        ImGuiEx::DebugLine(old_box[i].get_pos(), new_box[i].get_pos(), ImColor(0,255,0), 4.f); // green line to symbolize the desired path
        physics_component->add_force_to_move_to(new_box[i].get_pos(), SCREEN_SPACE, cfg->ui_node_speed, true);
    }
}

std::vector<NodeView *> NodeViewConstraint::clean(std::vector<NodeView *> &views)
{
    std::vector<NodeView *> result;
    for(auto* view : views)
    {
        if (view->visible())
            if (!view->pinned())
                result.push_back(view);
    }
    return result;
}

void NodeViewConstraint::draw_ui()
{
    if( ImGui::TreeNode(name) )
    {
        ImGui::Checkbox("enabled", &enabled);
        ImGui::TreePop();
    }
}
