#include "NodeViewConstraint.h"

#include <numeric>

#include "core/ForLoopNode.h"

#include "Nodable.h"
#include "NodeView.h"
#include "Physics.h"

using namespace ndbl;
using namespace fw;

NodeViewConstraint::NodeViewConstraint(const char* _name, ViewConstraint_t _type)
: m_type(_type)
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

    switch ( m_type )
    {
        case ViewConstraint_t::AlignOnBBoxLeft:
        {
            /*
             * Align first target's bbox border left with all driver's bbox border right
             */

            NodeView* target = clean_targets[0];

            if(!target->pinned() && target->is_visible())
            {
                ImRect drivers_bbox = NodeView::get_rect(clean_drivers, true);
                ImVec2 new_position(drivers_bbox.GetCenter()
                                    - ImVec2(drivers_bbox.GetSize().x * 0.5f
                                    + config.ui_node_spacing
                                    + target->get_rect().GetSize().x * 0.5f, 0 ));
                auto target_physics = target->get_owner()->get_component<Physics>();
                target_physics->add_force_to_translate_to(new_position + m_offset, config.ui_node_speed);
            }
            break;
        }

        case ViewConstraint_t::MakeRowAndAlignOnBBoxTop:
        case ViewConstraint_t::MakeRowAndAlignOnBBoxBottom:
        {
            /*
             * Make a row with targets, and constrain it to be above at the top (or bottom) the driver's bbox
             */
            NodeView* driver = clean_drivers[0];

            // Compute each size_x and size_x_total :
            //-----------------------

            std::vector<float> size_x;
            bool recursively = m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxBottom;

            for (auto each_target : clean_targets)
            {
                float size = 0.0f;
                if( !(each_target->pinned() || !each_target->is_visible()) )
                {
                    size = each_target->get_rect(recursively).GetSize().x;
                }
                size_x.push_back(size);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            ImVec2   driver_pos  = driver->get_position(fw::Space_Local);
            float    start_pos_x = driver_pos.x;

            if ( driver->get_owner()->is_instruction() && m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxTop )
            {
                start_pos_x += driver->get_size().x / 2.0f // indented
                             + config.ui_node_spacing;
            } else {
                start_pos_x -= size_x_total / 2.0f; // align horizontally on driver_pos.x
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto each_target : clean_targets)
            {
                if ( !each_target->pinned() && each_target->is_visible() )
                {
                    // Compute new position for this input view
                    float y_offset = config.ui_node_spacing
                                     + each_target->get_size().y / 2.0f
                                     + driver->get_size().y / 2.0f;

                    // Flip vertically
                    if(m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxTop ) y_offset *= -1.0f;

                    ImVec2 new_pos;
                    new_pos.x = start_pos_x + size_x[node_index] / 2.0f;
                    new_pos.y = driver_pos.y + y_offset;

                    if ( each_target->get_owner()->should_be_constrain_to_follow_output( driver->get_owner() )
                         || m_type != ViewConstraint_t::MakeRowAndAlignOnBBoxTop
                       )
                    {
                        auto target_physics = each_target->get_owner()->get_component<Physics>();
                        target_physics->add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
                        start_pos_x += size_x[node_index] + config.ui_node_spacing;
                    }
                    node_index++;
                }
            }
            break;
        }

        case ViewConstraint_t::FollowWithChildren:
        {
            /*
             * Constrain the target view (and its children) to follow the drivers' bbox
             */

            NodeView* target = clean_targets[0];
            if (!target->pinned() && target->is_visible() )
            {
                // compute
                auto drivers_rect = NodeView::get_rect(clean_drivers, false, true);

                auto target_rect  = target->get_rect(true, true);
                ImVec2 target_driver_offset(drivers_rect.Max - target_rect.Min);
                ImVec2 new_pos;
                new_pos.x = drivers_rect.GetCenter().x;
                new_pos.y = target->get_position(fw::Space_Local).y + target_driver_offset.y + config.ui_node_spacing;

                // apply
                auto target_physics = target->get_owner()->get_component<Physics>();
                target_physics->add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
                break;
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
        ImGui::Text("Type:     %s", to_string(m_type));
        ImGui::Text("Drivers:  %zu", m_drivers.size());
        ImGui::Text("Targets:  %zu", m_targets.size());
        ImGui::Checkbox("On/Off", &m_is_active);
        ImGui::TreePop();
    }
}
