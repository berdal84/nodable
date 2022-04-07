#include <nodable/app/NodeView.h>

#include <nodable/app/Settings.h>
#include <nodable/core/Node.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/app/IAppCtx.h>

#include <numeric>

using namespace Nodable;

NodeViewConstraint::NodeViewConstraint(IAppCtx& _ctx, NodeViewConstraint::Type _type)
: m_type(_type)
, m_ctx(_ctx)
, m_filter(always)
{

}

void NodeViewConstraint::apply(float _dt)
{
    if( !should_apply() )
    {
        return;
    }

    Settings& settings = m_ctx.settings();

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

    switch ( this->m_type )
    {
        case Type::AlignOnBBoxLeft:
        {
            if(!first_target->is_pinned() && first_target->is_visible())
            {
                ImRect bbox = NodeView::get_rect(clean_drivers, true);
                vec2 newPos(bbox.GetCenter() - vec2(bbox.GetSize().x * 0.5f + settings.ui_node_spacing +
                                                    first_target->get_rect().GetSize().x * 0.5f, 0 ));
                first_target->add_force_to_translate_to(newPos + m_offset, settings.ui_node_speed);
            }

            break;
        }

        case Type::AlignOnBBoxTop:
        {
            if(!first_target->is_pinned() && first_target->is_visible() && first_target->should_follow_output(clean_drivers[0]))
            {
                ImRect bbox = NodeView::get_rect(clean_drivers);
                vec2 newPos(bbox.GetCenter() + vec2(0.0, -bbox.GetHeight() * 0.5f - settings.ui_node_spacing));
                newPos.y -= settings.ui_node_spacing + first_target->get_size().y / 2.0f;
                newPos.x += settings.ui_node_spacing + first_target->get_size().x / 2.0f;

                if (newPos.y < first_target->get_position().y )
                    first_target->add_force_to_translate_to(newPos + m_offset, settings.ui_node_speed, true);
            }

            break;
        }

        case Type::MakeRowAndAlignOnBBoxTop:
        case Type::MakeRowAndAlignOnBBoxBottom:
        {
            // Compute size_x_total :
            //-----------------------

            std::vector<float> size_x;
            bool recursively = m_type == Type::MakeRowAndAlignOnBBoxBottom;

            for (auto each_target : clean_targets)
            {
                bool ignore = each_target->is_pinned() || !each_target->is_visible();
                size_x.push_back(ignore ? 0.f : each_target->get_rect(recursively).GetSize().x);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            float start_pos_x = first_driver->get_position().x - size_x_total / 2.0f;
            R::Class_ptr masterClass = first_driver->get_owner()->get_class();
            if (masterClass->is_child_of<InstructionNode>()
                || (masterClass->is_child_of<IConditionalStruct>() && m_type == Type::MakeRowAndAlignOnBBoxTop))
            {
                // indent
                start_pos_x = first_driver->get_position().x + first_driver->get_size().x / 2.0f + settings.ui_node_spacing;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto each_target : clean_targets)
            {
                if (!each_target->is_pinned() && each_target->is_visible() )
                {
                    // Compute new position for this input view
                    float verticalOffset = settings.ui_node_spacing + each_target->get_size().y / 2.0f +
                                           first_driver->get_size().y / 2.0f;
                    if(m_type == MakeRowAndAlignOnBBoxTop )
                    {
                        verticalOffset *= -1.0f;
                    }

                    vec2 new_pos = vec2(start_pos_x + size_x[node_index] / 2.0f,
                                        first_driver->get_position().y + verticalOffset);

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

        case Type::FollowWithChildren:
        {
            if (!first_target->is_pinned() && first_target->is_visible() )
            {
                // compute
                auto masterRect = NodeView::get_rect(clean_drivers, false, true);
                auto slaveRect = first_target->get_rect(true, true);
                vec2 slaveMasterOffset(masterRect.Max - slaveRect.Min);
                vec2 newPos(masterRect.GetCenter().x,
                            first_target->get_position().y + slaveMasterOffset.y + settings.ui_node_spacing);

                // apply
                first_target->add_force_to_translate_to(newPos + m_offset, settings.ui_node_speed, true);
                break;
            }
        }

        case Type::Follow:
        {
            if (!first_target->is_pinned() && first_target->is_visible() )
            {
                // compute
                vec2 newPos(clean_drivers[0]->get_position() + vec2(0.0f, clean_drivers[0]->get_size().y));
                newPos.y += settings.ui_node_spacing + first_target->get_size().y;

                // apply
                first_target->add_force_to_translate_to(newPos + m_offset, settings.ui_node_speed);
                break;
            }
        }
    }
}

void NodeViewConstraint::add_target(NodeView *_target)
{
    NODABLE_ASSERT(_target != nullptr);
    m_targets.push_back(_target);
}

void NodeViewConstraint::add_driver(NodeView *_driver)
{
    NODABLE_ASSERT(_driver != nullptr);
    m_drivers.push_back(_driver);
}

void NodeViewConstraint::add_targets(const std::vector<NodeView *> &_new_targets)
{
    m_targets.insert(m_targets.end(), _new_targets.begin(), _new_targets.end());
}

void NodeViewConstraint::add_drivers(const std::vector<NodeView *> &_new_drivers)
{
    m_drivers.insert(m_drivers.end(), _new_drivers.begin(), _new_drivers.end());
}


auto not_expanded  = [](const NodeView* _view ) { return !_view->is_expanded(); };

const NodeViewConstraint::Filter
        NodeViewConstraint::always = [](NodeViewConstraint* _constraint){ return true; };

const NodeViewConstraint::Filter
        NodeViewConstraint::no_target_expanded = [](const NodeViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_targets.cbegin(), _constraint->m_targets.cend(), not_expanded)
           == _constraint->m_targets.cend();
};

const NodeViewConstraint::Filter
        NodeViewConstraint::no_driver_expanded = [](const NodeViewConstraint* _constraint)
{
    return std::find_if(_constraint->m_drivers.cbegin(), _constraint->m_drivers.cend(), not_expanded)
           == _constraint->m_drivers.cend();
};

bool NodeViewConstraint::should_apply()
{
    return m_filter(this);
}
