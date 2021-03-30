#include "NodeTraversal.h"
#include "Wire.h"
#include "Log.h"
#include <algorithm>
#include "Node/ScopedCodeBlockNode.h"

using namespace Nodable;

Result NodeTraversal::update(Node* _rootNode)
{
    initialize();
    LOG_VERBOSE("NodeTraversal", "Update %s \n", _rootNode->getLabel() );
    auto result = updateRecursively(_rootNode);
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::Update done.\n");
    return result;
}

Result NodeTraversal::setDirty(Node* _rootNode)
{
    initialize();
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::setDirty %s \n", _rootNode->getLabel() );
    auto result = setDirtyRecursively(_rootNode);
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::setDirty done.\n");
    return result;
}

Result NodeTraversal::update(ScopedCodeBlockNode *_scope)
{
    initialize();
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::update %s \n", _scope->getLabel() );
    auto result = updateRecursively(_scope);
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::update done.\n");
    return result;
}

void NodeTraversal::initialize()
{
    this->stats = {};
}


Result NodeTraversal::setDirtyRecursively(Node* _node) {

    Result result;

    LOG_VERBOSE("NodeTraversal", "NodeTraversal::SetDirtyEx\n");

    if( !stats.hasBeenTraversed(_node) )
    {
        stats.traversed.push_back(_node);

        _node->setDirty();

        for (auto eachOutput : _node->getOutputs() )
        {
            auto r = setDirtyRecursively(eachOutput);
            if( r == Result::Failure )
                return Result::Failure;
        };

        for (auto& eachChild : _node->getChildren() )
        {
            auto r = setDirtyRecursively(eachChild);
            if( r == Result::Failure )
                return Result::Failure;
        }

        if( auto parent = _node->getParent())
        {
            setDirtyRecursively(parent);
        }

        result = Result::Success;
    } else {
        result = Result::Failure;
    }

    return result;
}

Result NodeTraversal::updateRecursively(Node* _node) {

    Result result;
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::UpdateEx\n");

    if( !stats.hasBeenTraversed(_node) )
    {
        // Evaluates only if dirty flag is on
        if (_node->isDirty(true))
        {
            stats.traversed.push_back(_node);

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
                    updateRecursively(sourceNode);

                    /* transfert the freshly updated value from source to target member */
                    wireTarget->set(wireSource);

                }
            }

            _node->update();

            for (auto& eachChild : _node->getChildren() )
            {
                updateRecursively(eachChild);
            }

        }
        result = Result::Success;

    } else {
        result = Result::Failure;
        LOG_WARNING("NodeTraversal", "Unable to update Node %s, cycle detected.\n", _node->getLabel() );
    }

    return result;
}

void NodeTraversal::logStats()
{
    LOG_MESSAGE("NodeTraversal", "traversed %i node(s).\n", (int)stats.traversed.size());
}

bool NodeTraversal::hasAChildDirty(const Node *_node)
{
    initialize();
    return hasAChildDirtyRec(_node);
}

bool NodeTraversal::hasAChildDirtyRec(const Node *_node)
{
    bool result = false;
    LOG_VERBOSE("NodeTraversal", "NodeTraversal::UpdateEx\n");

    if( !stats.hasBeenTraversed(_node) )
    {
        stats.traversed.push_back(_node);

        if( _node->getChildren().empty())
        {
            result = _node->isDirty();
        }
        else
        {
            for (auto& eachChild : _node->getChildren() )
            {
                result |= hasAChildDirtyRec(eachChild);
            }
        }

    } else {
        LOG_WARNING("NodeTraversal", "Unable to update Node %s, cycle detected.\n", _node->getLabel() );
    }

    return result;
}

bool Stats::hasBeenTraversed(const Node* _node) const
{
    return  std::find( traversed.cbegin(), traversed.cend(), _node ) != traversed.cend();
}
