#include <ndbl/gui/NodeView.h>

#include <numeric>

#include <ndbl/gui/Settings.h>
#include <ndbl/core/Node.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/ForLoopNode.h>

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
    if( !should_apply() )
    {
        return;
    }

    Settings& settings = Settings::get_instance();

    // try to get a visible view for drivers, is view is not visible we take the parent
    std::vector<NodeView*> clean_drivers;
    for(auto each : m_drivers)
    {
        clean_drivers.push_back(NodeView::substitute_with_parent_if_not_visible(each ) );
    }

    // the same for targets
    std::vector<NodeView*> clean_targets;
    for(auto each : m_targets)
    {
        clean_targets.push_back(NodeView::substitute_with_parent_if_not_visible(each ) );
    }

    // return if no views are visible
    auto is_visible = [](NodeView* view) { return view->is_visible(); };
    if (std::find_if(clean_targets.begin(), clean_targets.end(), is_visible) == clean_targets.end()) return;
    if (std::find_if(clean_drivers.begin(), clean_drivers.end(), is_visible) == clean_drivers.end()) return;

    // shortcuts
    auto first_target = clean_targets[0];
    auto first_driver = clean_drivers[0];

    switch ( m_type )
    {
        case ViewConstraint_t::AlignOnBBoxLeft:
        {
            if(!first_target->is_pinned() && first_target->is_visible())
            {
                ImRect bbox = NodeView::get_rect(clean_drivers, true);
                fw::vec2 newPos(bbox.GetCenter()
                            - fw::vec2(bbox.GetSize().x * 0.5f
                            + settings.ui_node_spacing
                            + first_target->get_rect().GetSize().x * 0.5f, 0 ));
                first_target->add_force_to_translate_to(newPos + m_offset, settings.ui_node_speed);
            }

            break;
        }

        case ViewConstraint_t::AlignOnBBoxTop:
        {
            if(!first_target->is_pinned() && first_target->is_visible() && first_target->should_follow_output(clean_drivers[0]))
            {
                ImRect bbox = NodeView::get_rect(clean_drivers);
                fw::vec2 newPos(bbox.GetCenter() + fw::vec2(0.0, -bbox.GetHeight() * 0.5f - settings.ui_node_spacing));
                newPos.y -= settings.ui_node_spacing + first_target->get_size().y / 2.0f;
                newPos.x += settings.ui_node_spacing + first_target->get_size().x / 2.0f;

                if (newPos.y < first_target->get_position().y )
                {
                    first_target->add_force_to_translate_to(newPos + m_offset, settings.ui_node_speed, true);
                }
            }

            break;
        }

        case ViewConstraint_t::MakeRowAndAlignOnBBoxTop:
        case ViewConstraint_t::MakeRowAndAlignOnBBoxBottom:
        {
            // Compute size_x_total :
            //-----------------------

            std::vector<float> size_x;
            bool recursively = m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxBottom;

            for (auto each_target : clean_targets)
            {
                bool ignore = each_target->is_pinned() || !each_target->is_visible();
                size_x.push_back(ignore ? 0.f : each_target->get_rect(recursively).GetSize().x);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            float        start_pos_x = first_driver->get_position().x - size_x_total / 2.0f;
            fw::type driver_type = first_driver->get_owner()->get_type();

            if (driver_type.is_child_of<InstructionNode>()
                || (driver_type.is_child_of<IConditionalStruct>() && m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxTop))
            {
                // indent
                start_pos_x = first_driver->get_position().x
                        + first_driver->get_size().x / 2.0f
                        + settings.ui_node_spacing;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto each_target : clean_targets)
            {
                if (!each_target->is_pinned() && each_target->is_visible() )
                {
                    // Compute new position for this input view
                    float y_offset = settings.ui_node_spacing
                            + each_target->get_size().y / 2.0f
                            + first_driver->get_size().y / 2.0f;

                    if(m_type == ViewConstraint_t::MakeRowAndAlignOnBBoxTop ) y_offset *= -1.0f;

                    fw::vec2 new_pos;
                    new_pos.x = start_pos_x + size_x[node_index] / 2.0f;
                    new_pos.y = first_driver->get_position().y + y_offset;

                    if (each_target->should_follow_output(first_driver) )
                    {
                        each_target->add_force_to_translate_to(new_pos + m_offset, settings.ui_node_speed, true);
                        start_pos_x += size_x[node_index] + settings.ui_node_spacing;
                    }
                    node_index++;
                }
            }
            break;
        }

        case ViewConstraint_t::FollowWithChildren:
        {
            if (!first_target->is_pinned() && first_target->is_visible() )
            {
                // compute
                auto drivers_rect = NodeView::get_rect(clean_drivers, false, true);
                auto target_rect  = first_target->get_rect(true, true);
                fw::vec2 target_driver_offset(drivers_rect.Max - target_rect.Min);
                fw::vec2 new_pos;
                new_pos.x = drivers_rect.GetCenter().x;
                new_pos.y = first_target->get_position().y + target_driver_offset.y + settings.ui_node_spacing;

                // apply
                first_target->add_force_to_translate_to(new_pos + m_offset, settings.ui_node_speed, true);
                break;
            }
        }

        case ViewConstraint_t::Follow:
        {
            if (!first_target->is_pinned() && first_target->is_visible() )
            {
                // compute
                fw::vec2 new_pos = clean_drivers[0]->get_position();
                new_pos     += fw::vec2(0.0f, clean_drivers[0]->get_size().y);
                new_pos.y   += settings.ui_node_spacing + first_target->get_size().y;

                // apply
                first_target->add_force_to_translate_to(new_pos + m_offset, settings.ui_node_speed);
                break;
            }
        }
    }
}

void ViewConstraint::add_target(NodeView *_target)
{
    NDBL_ASSERT(_target != nullptr);
    m_targets.push_back(_target);
}

void ViewConstraint::add_driver(NodeView *_driver)
{
    NDBL_ASSERT(_driver != nullptr);
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

bool ViewConstraint::should_apply()
{
    return m_is_enable && m_filter(this);
}

void ViewConstraint::draw_view()
{
    if( ImGui::TreeNode(m_name) )
    {
        ImGui::Text("Type:     %s", to_string(m_type));
        ImGui::Text("Drivers:  %lu", m_drivers.size());
        ImGui::Text("Targets:  %lu", m_targets.size());
        ImGui::Checkbox("Enable", &m_is_enable);
        ImGui::TreePop();
    }
}
