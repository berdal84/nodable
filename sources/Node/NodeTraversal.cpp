#include "NodeTraversal.h"
#include "Wire.h"
#include "Log.h"
#include <algorithm>

using namespace Nodable;

Result NodeTraversal::Update(Node* _rootNode) {
    LOG_MESSAGE(Log::Verbosity::Verbose, "NodeTraversal::Update %s \n", _rootNode->getLabel() );
    std::vector<Node*> traversed;
    auto result = NodeTraversal::UpdateRecursively(_rootNode, traversed);
    LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "NodeTraversal::Update done.\n");
    return result;
}

Result NodeTraversal::SetDirty(Node* _rootNode) {
    LOG_MESSAGE(Log::Verbosity::Verbose, "NodeTraversal::SetDirty %s \n", _rootNode->getLabel() );
    std::vector<Node*> traversed;
    auto result = NodeTraversal::SetDirtyRecursively(_rootNode, traversed);
    LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "NodeTraversal::SetDirty done.\n");
    return result;
}

Result NodeTraversal::SetDirtyRecursively(Node* _node, std::vector<Node*>& _traversed) {

    Result result;
    
    LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "NodeTraversal::SetDirtyEx\n");
    
    // Check if we already updated this node
    auto alreadyUpdated = std::find( _traversed.cbegin(), _traversed.cend(), _node ) != _traversed.cend();
    if( !alreadyUpdated )
    {
        _traversed.push_back(_node);

        _node->setDirty();

        for (auto wire : _node->getWires() )
        {

            if (wire->getSource()->getOwner() == _node &&
                wire->getTarget() != nullptr)
            {
                auto targetNode = reinterpret_cast<Node*>(wire->getTarget()->getOwner());
                
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

Result NodeTraversal::UpdateRecursively(Node* _node, std::vector<Node*>& _traversed) {
    
    Result result;
    LOG_MESSAGE(Log::Verbosity::ExtraVerbose, "NodeTraversal::UpdateEx\n");
    
    // Check if we already updated this node
    auto alreadyUpdated = std::find( _traversed.cbegin(), _traversed.cend(), _node ) != _traversed.cend();
    if( !alreadyUpdated )
    {
        _traversed.push_back(_node);

        // Evaluates only if dirty flag is on
        if (_node->isDirty())
        {
            // first we need to evaluate each input and transmit its results thru the wire
            auto wires = _node->getWires();
            for (auto wire : wires)
            {
                auto wireTarget = wire->getTarget();
                auto wireSource = wire->getSource();

                if ( _node->has(wireTarget) &&
                    wireSource != nullptr) 
                {
                    /* update the source entity */
                    auto sourceNode = reinterpret_cast<Node*>(wireSource->getOwner());
                    NodeTraversal::UpdateRecursively(sourceNode, _traversed);
                    
                    /* transfert the freshly updated value from source to target member */
                    wireTarget->set(wireSource);

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
        LOG_WARNING(Log::Verbosity::Normal, "Unable to update Node %s, cycle detected.", _node->getLabel() );
    }

    return result;
}