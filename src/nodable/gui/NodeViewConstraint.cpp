#include "NodeViewConstraint.h"

#include <numeric>

#include "core/ForLoopNode.h"

#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"

using namespace ndbl;
using namespace fw;

NodeViewConstraint::NodeViewConstraint(const char* _name, ConstrainFlags _flags)
: m_flags(_flags)
, m_filter(always)
, m_is_active(true)
, m_name(_name)
{
}

void NodeViewConstraint::apply(float _dt)
{
    bool should_apply = m_is_active && m_filter(this);
    if(!should_apply)
    {
        return;
    }

    /*
     * To get a clean list of node views.
     * Substitute each not visible view by their respective parent.
     */
    auto get_clean = [](std::vector<NodeView*> _in)
    {
        std::vector<NodeView*> out;
        out.reserve(_in.size());
        for(auto each : _in)
        {
            out.push_back(NodeView::substitute_with_parent_if_not_visible(each));
        }
        return std::move(out);
    };

    std::vector<NodeView*> clean_drivers = get_clean( Pool::get_pool()->get( m_drivers ) );
    std::vector<NodeView*> clean_targets = get_clean( Pool::get_pool()->get( m_targets ) );

    //debug
    if( fw::ImGuiEx::debug )
    {
        for (auto each_target: clean_targets)
        {
            for (auto each_driver: clean_drivers)
            {
                fw::ImGuiEx::DebugLine(
                        each_driver->get_position(fw::Space_Screen),
                        each_target->get_position(fw::Space_Screen),
                        IM_COL32(0, 0, 255, 30), 1.0f);
            }
        }
    }

    auto none_is_visible = [](const std::vector<NodeView*>& _views)-> bool {
        auto is_visible = [](const NodeView* view) { return view->is_visible(); };
        return std::find_if(_views.begin(), _views.end(), is_visible) == _views.end();
    };
    if (none_is_visible(clean_targets) || none_is_visible(clean_drivers)) return;

    const Config& config = Nodable::get_instance().config;

    switch ( m_flags & ConstrainFlag_LAYOUT_MASK )
    {
        case ConstrainFlag_LAYOUT_MAKE_ROW:
        {
            /*
             * Make a row with targets, and constrain it to be above at the top (or bottom) the driver's bbox
             */
            NodeView*   driver            = clean_drivers[0];
            const bool  align_bbox_bottom = m_flags & ConstrainFlag_ALIGN_BBOX_BOTTOM;
            const float y_direction       = align_bbox_bottom ? 1.0f : -1.0f;
            float       size_x_total      = 0.0f;
            ImVec2      driver_pos        = driver->get_position(fw::Space_Local);
            ImVec2      start_pos         = driver_pos;
            const Node& driver_owner      = *driver->get_owner();
            std::vector<ImVec2> target_sizes;

            // Compute each target_sizes and size_x_total :
            //-----------------------
            for (auto each_target : clean_targets)
            {
                ImVec2 size;
                if( !(each_target->pinned() || !each_target->is_visible()) )
                {
                    size = each_target->get_rect( align_bbox_bottom ).GetSize();
                }
                target_sizes.push_back(size);
                size_x_total += size.x;
            }

            // Determine x position start:
            //---------------------------

            // x alignment
            //
            // We add an indentation when driver is an instruction without being connected to a predecessor
            if ( driver_owner.is_instruction() && !driver_owner.predecessors().empty() && not align_bbox_bottom )
            {
                start_pos.x += driver->get_size().x / 4.0f
                             + config.ui_node_spacing;

            // Otherwise we simply align vertically
            } else {
                start_pos.x -= size_x_total / 2.0f;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;

            float y_offset = config.ui_node_spacing + driver->get_size().y / 2.0f;
            start_pos.y += y_offset * y_direction;

            for (auto each_target : clean_targets)
            {
                if ( !each_target->pinned() && each_target->is_visible() )
                {
                    // Compute new position for this input view
                    ImVec2 new_pos(
                        start_pos.x + target_sizes[node_index].x / 2.0f + config.ui_node_spacing,
                        start_pos.y + y_direction * target_sizes[node_index].y / 2.0f + config.ui_node_spacing
                    );

                    const Node& target_owner = *each_target->get_owner();
                    const bool constrained = target_owner.should_be_constrain_to_follow_output( driver_owner.poolid() );
                    if ( constrained || align_bbox_bottom )
                    {
                        auto target_physics = target_owner.get_component<Physics>();
                        target_physics->add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
                        start_pos.x += target_sizes[node_index].x + config.ui_node_spacing;
                        // start_pos.y += y_direction * (target_sizes[node_index].y + config.ui_node_spacing);
                    }
                    node_index++;
                }
            }
            break;
        }

        case ConstrainFlag_LAYOUT_DEFAULT:
        {
            NodeView* target = clean_targets[0];
            if (!target->pinned() && target->is_visible() )
            {
                Physics& target_physics = *target->get_owner()->get_component<Physics>();

                if( m_flags & ConstrainFlag_LAYOUT_FOLLOW_WITH_CHILDREN )
                {
                    /*
                    * Constrain the target view (and its children) to follow the drivers' bbox
                    */

                    // compute
                    auto drivers_rect = NodeView::get_rect(clean_drivers, false);

                    auto target_rect  = target->get_rect(true, true);
                    ImVec2 target_driver_offset(drivers_rect.Max - target_rect.Min);
                    ImVec2 new_pos;
                    new_pos.x = drivers_rect.GetCenter().x;
                    new_pos.y = target->get_position(fw::Space_Local).y + target_driver_offset.y + config.ui_node_spacing;

                    // apply
                    target_physics.add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
                }
                else
                {
                    /*
                     * Align first target's bbox border left with all driver's bbox border right
                     */
                    ImRect drivers_bbox = NodeView::get_rect(clean_drivers, true);
                    ImVec2 new_position(drivers_bbox.GetCenter()
                                         - ImVec2(drivers_bbox.GetSize().x * 0.5f
                                         + config.ui_node_spacing
                                         + target->get_rect().GetSize().x * 0.5f, 0 ));
                    target_physics.add_force_to_translate_to(new_position + m_offset, config.ui_node_speed);
                }
            }
        }
    }
}

void NodeViewConstraint::add_target(PoolID<NodeView> _target)
{
    FW_ASSERT( _target );
    m_targets.push_back(_target);
}

void NodeViewConstraint::add_driver(PoolID<NodeView> _driver)
{
    FW_ASSERT( _driver );
    m_drivers.push_back(_driver);
}

void NodeViewConstraint::add_targets(const std::vector<PoolID<NodeView>> &_new_targets)
{
    m_targets.insert(m_targets.end(), _new_targets.begin(), _new_targets.end());
}

void NodeViewConstraint::add_drivers(const std::vector<PoolID<NodeView>> &_new_drivers)
{
    m_drivers.insert(m_drivers.end(), _new_drivers.begin(), _new_drivers.end());
}


auto not_expanded  = [](PoolID<const NodeView> _view ) { return !_view->is_expanded(); };

const NodeViewConstraint::Filter
        NodeViewConstraint::always = [](NodeViewConstraint* _constraint){ return true; };

const NodeViewConstraint::Filter
        NodeViewConstraint::no_target_expanded = [](const NodeViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_targets.cbegin(), _constraint->m_targets.cend(), not_expanded)
           == _constraint->m_targets.cend();
};

const NodeViewConstraint::Filter
        NodeViewConstraint::drivers_are_expanded = [](const NodeViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_drivers.cbegin(), _constraint->m_drivers.cend(), not_expanded)
           == _constraint->m_drivers.cend();
};

void NodeViewConstraint::draw_view()
{
    if( ImGui::TreeNode(m_name) )
    {
        ImGui::Text("Type:     %s", to_string( (ConstrainFlag_)m_flags ));
        ImGui::Text("Drivers:  %zu", m_drivers.size());
        ImGui::Text("Targets:  %zu", m_targets.size());
        ImGui::Checkbox("On/Off", &m_is_active);
        ImGui::TreePop();
    }
}
