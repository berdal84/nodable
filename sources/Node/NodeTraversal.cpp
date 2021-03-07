#include "NodeTraversal.h"
#include "Wire.h"
#include "Log.h"
#include <algorithm>

using namespace Nodable;

Result NodeTraversal::Update(Node* _rootNode) {
    LOG_VERBOSE("NodeTraversal", "Update %s \n", _rootNode->getLabel() );
    std::vector<Node*> traversed;
    auto result = NodeTraversal::UpdateRecursively(_rootNode, traversed);
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::Update done.\n");
    return result;
}

Result NodeTraversal::SetDirty(Node* _rootNode) {
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::SetDirty %s \n", _rootNode->getLabel() );
    std::vector<Node*> traversed;
    auto result = NodeTraversal::SetDirtyRecursively(_rootNode, traversed);
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::SetDirty done.\n");
    return result;
}

Result NodeTraversal::SetDirtyRecursively(Node* _node, std::vector<Node*>& _traversed) {

    Result result;

    LOG_VERBOSE("NodeTraversal", "NodeTraversal::SetDirtyEx\n");
    
    // Check if we already updated this node
    auto alreadyUpdated = std::find( _traversed.cbegin(), _traversed.cend(), _node ) != _traversed.cend();
    if( !alreadyUpdated )
    {
        _traversed.push_back(_node);

        _node->setDirty();

        for (auto eachOutput : _node->getOutputs() )
        {
            auto r = NodeTraversal::SetDirtyRecursively(eachOutput, _traversed);
            if( r == Result::Failure )
                return Result::Failure;
        };

        result = Result::Success;
    } else {
        result = Result::Failure;
    }

    return result;
}

Result NodeTraversal::UpdateRecursively(Node* _node, std::vector<Node*>& _traversed) {
    
    Result result;
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::UpdateEx\n");
    
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

            _node->update();

        }
        result = Result::Success;

    } else {
        result = Result::Failure;
        LOG_WARNING("NodeTraversal", "Unable to update Node %s, cycle detected.", _node->getLabel() );
    }

    return result;
}