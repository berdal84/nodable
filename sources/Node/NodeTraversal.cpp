#include "NodeTraversal.h"
#include "Wire.h"

#include <iostream>
#include <Core/Log.h>

using namespace Nodable;

Result NodeTraversal::Update(const std::shared_ptr<Node>&  _rootNode) {
    LOG_MESSAGE(1u, "NodeTraversal::Update %s \n", _rootNode->getLabel() );
    std::vector<std::shared_ptr<Node>> traversed;
    auto result = NodeTraversal::UpdateRecursively(_rootNode, traversed);
    LOG_MESSAGE(2u, "NodeTraversal::Update done.\n");
    return result;
}

Result NodeTraversal::SetDirty(const std::shared_ptr<Node>&  _rootNode) {
    LOG_MESSAGE(1u, "NodeTraversal::SetDirty %s \n", _rootNode->getLabel() );
    std::vector<std::shared_ptr<Node>> traversed;
    auto result = NodeTraversal::SetDirtyRecursively(_rootNode, traversed);
    LOG_MESSAGE(2u, "NodeTraversal::SetDirty done.\n");
    return result;
}

Result NodeTraversal::SetDirtyRecursively(const std::shared_ptr<Node>& _node, std::vector<std::shared_ptr<Node>>& _traversed) {

    Result result;
    
    LOG_MESSAGE(2u, "NodeTraversal::SetDirtyEx\n");
    
    // Check if we already updated this node
    auto alreadyUpdated = std::find( _traversed.cbegin(), _traversed.cend(), _node ) != _traversed.cend();
    if( !alreadyUpdated )
    {
        LOG_MESSAGE(2u, "NodeTraversal::SetDirtyEx - not alreadyUpdated\n");
        _traversed.push_back(_node);

        _node->setDirty();

        for (const auto& wire : _node->getWires() )
        {
            LOG_MESSAGE(2u, "NodeTraversal::SetDirtyEx - eachWire...\n");

            if (wire->getSource()->getOwner() == _node &&
                wire->getTarget() != nullptr)
            {
                auto targetNode = std::static_pointer_cast<Node>( wire->getTarget()->getOwner() );

                LOG_MESSAGE(2u, "NodeTraversal::SetDirtyEx - recursive call !\n");
                auto r = NodeTraversal::SetDirtyRecursively(targetNode, _traversed);
                if( r == Result::Failure )
                    return Result::Failure;
            }
        };

        result = Result::Success;
    } else {
        result = Result::Failure;
    }

    return result;
}

Result NodeTraversal::UpdateRecursively(const std::shared_ptr<Node>& _node, std::vector<std::shared_ptr<Node>>& _traversed) {
    
    Result result;
    LOG_MESSAGE(2u, "NodeTraversal::UpdateEx\n");
    
    // Check if we already updated this node
    auto alreadyUpdated = std::find( _traversed.cbegin(), _traversed.cend(), _node ) != _traversed.cend();
    if( !alreadyUpdated )
    {
        LOG_MESSAGE(2u, "NodeTraversal::UpdateEx - NOT alreadyUpdated !\n");
        _traversed.push_back(_node);

        // Evaluates only if dirty flag is on
        if (_node->isDirty())
        {
            LOG_MESSAGE(2u, "NodeTraversal::UpdateEx - isDirty !\n");
            // first we need to evaluate each input and transmit its results thru the wire
            auto wires = _node->getWires();
            for (const auto& eachWire : wires)
            {
                LOG_MESSAGE(2u, "NodeTraversal::UpdateEx - eachWire...\n");
                auto wireTarget = eachWire->getTarget();
                auto wireSource = eachWire->getSource();

                if ( _node->has(wireTarget) &&
                    wireSource != nullptr) 
                {
                    /* update the source entity */
                    auto sourceNode = std::static_pointer_cast<Node>( wireSource->getOwner() );
                    NodeTraversal::UpdateRecursively(sourceNode, _traversed);
                    
                    /* transfert the freshly updated value from source to target member */
                    wireTarget->updateValueFromInputMemberValue();
                }
            }

            // Update
            _node->update();
            _node->setDirty(false);
       
            /* Set dirty all childrens
            for (auto wire : wires)
            {

                if (wire->getSource()->getOwner() == _node &&
                    wire->getTarget() != nullptr)
                {
                    auto targetNode = reinterpret_cast<Node*>(wire->getTarget()->getOwner());
                    NodeTraversal::SetDirty(targetNode);
                }
            };*/


        }
        result = Result::Success;

    } else {
        result = Result::Failure;
        LOG_WARNING(0u, "Unable to update Node %s, cycle detected.", _node->getLabel() );
    }

    return result;
}