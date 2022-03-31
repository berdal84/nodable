#include <nodable/app/NodeView.h>

#include <nodable/app/Settings.h>
#include <nodable/core/Node.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/app/AppContext.h>

#include <numeric>

using namespace Nodable;

NodeViewConstraint::NodeViewConstraint(const AppContext* _ctx, NodeViewConstraint::Type _type): m_type(_type), m_context(_ctx) {}

void NodeViewConstraint::apply(float _dt)
{
    Settings* settings = m_context->settings;

    auto is_visible = [](NodeView* view) { return view->isVisible(); };
    if (std::find_if(m_masters.begin(), m_masters.end(), is_visible) == m_masters.end()) return;
    if (std::find_if(m_slaves.begin(), m_slaves.end(), is_visible) == m_slaves.end()) return;

    auto master = m_masters.at(0);

    switch ( this->m_type )
    {
        case Type::AlignOnBBoxLeft:
        {
            auto slave = m_slaves.at(0);
            if(!slave->is_pinned() && slave->isVisible())
            {
                ImRect bbox = NodeView::get_rect(m_masters, true);
                vec2 newPos(bbox.GetCenter() - vec2(bbox.GetSize().x * 0.5f + settings->ui_node_spacing +
                                                            slave->get_rect().GetSize().x * 0.5f, 0 ));
                slave->add_force_to_translate_to(newPos + m_offset, settings->ui_node_speed);
            }

            break;
        }

        case Type::AlignOnBBoxTop:
        {
            auto slave = m_slaves.at(0);
            if(!slave->is_pinned() && slave->isVisible() && slave->should_follow_output(master))
            {
                ImRect bbox = NodeView::get_rect(m_masters);
                vec2 newPos(bbox.GetCenter() + vec2(0.0, -bbox.GetHeight() * 0.5f - settings->ui_node_spacing));
                newPos.y -= settings->ui_node_spacing + slave->get_size().y / 2.0f;
                newPos.x += settings->ui_node_spacing + slave->get_size().x / 2.0f;

                if ( newPos.y < slave->get_position().y )
                    slave->add_force_to_translate_to(newPos + m_offset, settings->ui_node_speed, true);
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
            for (auto eachSlave : m_slaves)
            {
                bool ignore = eachSlave->is_pinned() || !eachSlave->isVisible();
                size_x.push_back( ignore ? 0.f : eachSlave->get_rect(recursively).GetSize().x);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            float start_pos_x = master->get_position().x - size_x_total / 2.0f;
            R::Class_ptr masterClass = master->get_owner()->get_class();
            if (masterClass->is_child_of<InstructionNode>()
                || (masterClass->is_child_of<IConditionalStruct>() && m_type == Type::MakeRowAndAlignOnBBoxTop))
            {
                // indent
                start_pos_x = master->get_position().x + master->get_size().x / 2.0f + settings->ui_node_spacing;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto eachSlave : m_slaves)
            {
                if (!eachSlave->is_pinned() && eachSlave->isVisible() )
                {
                    // Compute new position for this input view
                    float verticalOffset = settings->ui_node_spacing + eachSlave->get_size().y / 2.0f +
                            master->get_size().y / 2.0f;
                    if(m_type == MakeRowAndAlignOnBBoxTop )
                    {
                        verticalOffset *= -1.0f;
                    }

                    vec2 new_pos = vec2(start_pos_x + size_x[node_index] / 2.0f,
                                        master->get_position().y + verticalOffset);

                    if (eachSlave->should_follow_output(master) )
                    {
                        eachSlave->add_force_to_translate_to(new_pos + m_offset, settings->ui_node_speed, true);
                        start_pos_x += size_x[node_index] + settings->ui_node_spacing;
                    }
                    node_index++;
                }
            }
            break;
        }

        case Type::FollowWithChildren:
        {
            auto slave = m_slaves.at(0);
            if (!slave->is_pinned() && slave->isVisible() )
            {
                // compute
                auto masterRect = NodeView::get_rect(m_masters, false, true);
                auto slaveRect = slave->get_rect(true, true);
                vec2 slaveMasterOffset(masterRect.Max - slaveRect.Min);
                vec2 newPos(masterRect.GetCenter().x,
                            slave->get_position().y + slaveMasterOffset.y + settings->ui_node_spacing);

                // apply
                slave->add_force_to_translate_to(newPos + m_offset, settings->ui_node_speed, true);
                break;
            }
        }

        case Type::Follow:
        {
            auto slave = m_slaves.at(0);
            if (!slave->is_pinned() && slave->isVisible() )
            {
                // compute
                vec2 newPos(master->get_position() + vec2(0.0f, master->get_size().y));
                newPos.y += settings->ui_node_spacing + slave->get_size().y;

                // apply
                slave->add_force_to_translate_to(newPos + m_offset, settings->ui_node_speed);
                break;
            }
        }
    }
}

void NodeViewConstraint::addSlave(NodeView *_subject) {
    NODABLE_ASSERT(_subject != nullptr);
    this->m_slaves.push_back(_subject);
}

void NodeViewConstraint::addMaster(NodeView *_subject) {
    NODABLE_ASSERT(_subject != nullptr);
    this->m_masters.push_back(_subject);
}

void NodeViewConstraint::addSlaves(const std::vector<NodeView *> &vector)
{
    for(auto each : vector)
        addSlave(each);
}

void NodeViewConstraint::addMasters(const std::vector<NodeView *> &vector)
{
    for(auto each : vector)
        addMaster(each);
}

