#include "NodeView.h"

#include <numeric>

#include "core/ForLoopNode.h"
#include "core/InstructionNode.h"

#include "Nodable.h"

using namespace ndbl;

ViewConstraint::ViewConstraint(const char* _name, ViewConstraint_t _type)
: m_type(_type)
, m_filter(always)
, m_is_enable(true)
, m_name(_name)
{
}

void ViewConstraint::apply(float _dt)
{
    bool should_apply = m_is_enable && m_filter(this);
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
            out.push_back(NodeView::substitute_with_parent_if_not_visible(each ));
        }
        return std::move(out);
    };

    std::vector<NodeView*> clean_drivers = get_clean(m_drivers);
    std::vector<NodeView*> clean_targets = get_clean(m_targets);

    //debug
    if( fw::ImGuiEx::debug )
    {
        for (auto each_target: clean_targets)
        {
            for (auto each_driver: clean_drivers)
            {
                fw::ImGuiEx::DebugLine(
                        each_driver->get_position(fw::Space_Local),
                        each_target->get_position(fw::Space_Local),
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

            if(!target->pinned && target->is_visible())
            {
                ImRect drivers_bbox = NodeView::get_rect(reinterpret_cast<const std::vector<const NodeView*> *>(&clean_drivers), true);
                ImVec2 new_position(drivers_bbox.GetCenter()
                                    - ImVec2(drivers_bbox.GetSize().x * 0.5f
                                    + config.ui_node_spacing
                                    + target->get_rect().GetSize().x * 0.5f, 0 ));
                target->add_force_to_translate_to(new_position + m_offset, config.ui_node_speed);
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

            // Compute size_x_total :
            //-----------------------

            std::vector<float> size_x;
            bool recursively = m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxBottom;

            for (auto each_target : clean_targets)
            {
                bool ignore = each_target->pinned || !each_target->is_visible();
                size_x.push_back(ignore ? 0.f : each_target->get_rect(recursively).GetSize().x);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            ImVec2   first_driver_pos = driver->get_position(fw::Space_Local);
            float    start_pos_x      = first_driver_pos.x - size_x_total / 2.0f;
            const fw::type* driver_type = driver->get_owner()->get_type();

            if (driver_type->is_child_of<InstructionNode>()
                || (driver_type->is_child_of<IConditionalStruct>() && m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxTop))
            {
                // indent
                start_pos_x = first_driver_pos.x
                              + driver->get_size().x / 2.0f
                              + config.ui_node_spacing;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto each_target : clean_targets)
            {
                if (!each_target->pinned && each_target->is_visible() )
                {
                    // Compute new position for this input view
                    float y_offset = config.ui_node_spacing
                                     + each_target->get_size().y / 2.0f
                                     + driver->get_size().y / 2.0f;

                    // top or bottom ?
                    if(m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxTop ) y_offset *= -1.0f;

                    ImVec2 new_pos;
                    new_pos.x = start_pos_x + size_x[node_index] / 2.0f;
                    new_pos.y = first_driver_pos.y + y_offset;

                    if (each_target->should_follow_output(driver) )
                    {
                        each_target->add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
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
            if (!target->pinned && target->is_visible() )
            {
                // compute
                auto drivers_rect = NodeView::get_rect(reinterpret_cast<const std::vector<const NodeView *> *>(&clean_drivers), false, true);
                auto target_rect  = target->get_rect(true, true);
                ImVec2 target_driver_offset(drivers_rect.Max - target_rect.Min);
                ImVec2 new_pos;
                new_pos.x = drivers_rect.GetCenter().x;
                new_pos.y = target->get_position(fw::Space_Local).y + target_driver_offset.y + config.ui_node_spacing;

                // apply
                target->add_force_to_translate_to(new_pos + m_offset, config.ui_node_speed, true);
                break;
            }
        }
    }
}

void ViewConstraint::add_target(NodeView *_target)
{
    FW_ASSERT(_target != nullptr);
    m_targets.push_back(_target);
}

void ViewConstraint::add_driver(NodeView *_driver)
{
    FW_ASSERT(_driver != nullptr);
    m_drivers.push_back(_driver);
}

void ViewConstraint::add_targets(const std::vector<NodeView *> &_new_targets)
{
    m_targets.insert(m_targets.end(), _new_targets.begin(), _new_targets.end());
}

void ViewConstraint::add_drivers(const std::vector<NodeView *> &_new_drivers)
{
    m_drivers.insert(m_drivers.end(), _new_drivers.begin(), _new_drivers.end());
}


auto not_expanded  = [](const NodeView* _view ) { return !_view->is_expanded(); };

const ViewConstraint::Filter
        ViewConstraint::always = [](ViewConstraint* _constraint){ return true; };

const ViewConstraint::Filter
        ViewConstraint::no_target_expanded = [](const ViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_targets.cbegin(), _constraint->m_targets.cend(), not_expanded)
           == _constraint->m_targets.cend();
};

const ViewConstraint::Filter
        ViewConstraint::drivers_are_expanded = [](const ViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_drivers.cbegin(), _constraint->m_drivers.cend(), not_expanded)
           == _constraint->m_drivers.cend();
};

void ViewConstraint::draw_view()
{
    if( ImGui::TreeNode(m_name) )
    {
        ImGui::Text("Type:     %s", to_string(m_type));
        ImGui::Text("Drivers:  %zu", m_drivers.size());
        ImGui::Text("Targets:  %zu", m_targets.size());
        ImGui::Checkbox("Enable", &m_is_enable);
        ImGui::TreePop();
    }
}
