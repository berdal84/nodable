#include <nodable/NodeView.h>

#include <nodable/Settings.h>
#include <nodable/Node.h>
#include <nodable/InstructionNode.h>
#include <nodable/ConditionalStructNode.h>

#include <numeric>

using namespace Nodable;

NodeViewConstraint::NodeViewConstraint(NodeViewConstraint::Type _type): type(_type) {}

void NodeViewConstraint::apply(float _dt) {

    auto is_visible = [](NodeView* view) { return view->isVisible(); };
    if ( std::find_if(masters.begin(), masters.end(), is_visible) == masters.end()) return;
    if ( std::find_if(slaves.begin(), slaves.end(), is_visible) == slaves.end()) return;

    auto settings = Settings::GetCurrent();

    LOG_VERBOSE("ViewConstraint", "applying constraint\n");
    auto master = masters.at(0);

    switch ( this->type )
    {
        case Type::AlignOnBBoxLeft:
        {
            auto slave = slaves.at(0);
            if( !slave->isPinned() && slave->isVisible())
            {
                ImRect bbox = NodeView::GetRect(masters, true);
                ImVec2 newPos(bbox.GetCenter() - ImVec2(bbox.GetSize().x * 0.5 + settings->ui.node.spacing + slave->getRect().GetSize().x * 0.5, 0 ));
                slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed);
            }

            break;
        }

        case Type::AlignOnBBoxTop:
        {
            auto slave = slaves.at(0);
            if( !slave->isPinned() && slave->isVisible() && slave->shouldFollowOutput(master))
            {
                ImRect bbox = NodeView::GetRect(masters);
                ImVec2 newPos(bbox.GetCenter() + ImVec2(0.0, -bbox.GetHeight() * 0.5f - settings->ui.node.spacing));
                newPos.y -= settings->ui.node.spacing + slave->getSize().y / 2.0f;
                newPos.x += settings->ui.node.spacing + slave->getSize().x / 2.0f;

                if ( newPos.y < slave->getPos().y )
                    slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed, true);
            }

            break;
        }

        case Type::MakeRowAndAlignOnBBoxTop:
        case Type::MakeRowAndAlignOnBBoxBottom:
        {
            // Compute size_x_total :
            //-----------------------

            std::vector<float> size_x;
            bool recursively = type == Type::MakeRowAndAlignOnBBoxBottom;
            for (auto eachSlave : slaves)
            {
                bool ignore = eachSlave->isPinned() || !eachSlave->isVisible();
                size_x.push_back( ignore ? 0.f : eachSlave->getRect(recursively).GetSize().x);
            }
            auto size_x_total = std::accumulate(size_x.begin(), size_x.end(), 0.0f);

            // Determine x position start:
            //---------------------------

            float start_pos_x = master->getPos().x - size_x_total / 2.0f;
            auto masterClass = master->getOwner()->getClass();
            if (masterClass == mirror::GetClass<InstructionNode>() ||
                ( masterClass == mirror::GetClass<ConditionalStructNode>() && type == Type::MakeRowAndAlignOnBBoxTop))
            {
                // indent
                start_pos_x = master->getPos().x + master->getSize().x / 2.0f + settings->ui.node.spacing;
            }

            // Constraint in row:
            //-------------------
            auto node_index = 0;
            for (auto eachSlave : slaves)
            {
                if (!eachSlave->isPinned() && eachSlave->isVisible() )
                {
                    // Compute new position for this input view
                    float verticalOffset = settings->ui.node.spacing + eachSlave->getSize().y / 2.0f + master->getSize().y / 2.0f;
                    if( type == MakeRowAndAlignOnBBoxTop )
                    {
                        verticalOffset *= -1.0f;
                    }

                    ImVec2 new_pos = ImVec2(start_pos_x + size_x[node_index] / 2.0f, master->getPos().y + verticalOffset);

                    if ( !eachSlave->shouldFollowOutput(master) )
                        new_pos.y = eachSlave->getPos().y; // remove constraint on Y axis

                    eachSlave->addForceToTranslateTo(new_pos + m_offset, _dt * settings->ui.node.speed, true);

                    start_pos_x += size_x[node_index] + settings->ui.node.spacing;
                    node_index++;
                }
            }
            break;
        }

        case Type::FollowWithChildren:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() && slave->isVisible() )
            {
                // compute
                auto masterRect = NodeView::GetRect(masters,false, true);
                auto slaveRect = slave->getRect(true,true );
                ImVec2 slaveMasterOffset(masterRect.Max - slaveRect.Min);
                ImVec2 newPos(masterRect.GetCenter().x,slave->getPos().y + slaveMasterOffset.y + settings->ui.node.spacing);

                // apply
                slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed, true);
                break;
            }
        }

        case Type::Follow:
        {
            auto slave = slaves.at(0);
            if ( !slave->isPinned() && slave->isVisible() )
            {
                // compute
                ImVec2 newPos(master->getPos() + ImVec2(0.0f, master->getSize().y));
                newPos.y += settings->ui.node.spacing + slave->getSize().y;

                // apply
                slave->addForceToTranslateTo(newPos + m_offset, _dt * settings->ui.node.speed);
                break;
            }
        }
    }
}

void NodeViewConstraint::addSlave(NodeView *_subject) {
    NODABLE_ASSERT(_subject != nullptr);
    this->slaves.push_back(_subject);
}

void NodeViewConstraint::addMaster(NodeView *_subject) {
    NODABLE_ASSERT(_subject != nullptr);
    this->masters.push_back(_subject);
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

