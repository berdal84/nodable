#include "GraphTraversal.h"

#include <algorithm>

#include "Core/Wire.h"
#include "Core/Log.h"
#include "Node/ScopedCodeBlockNode.h"
#include "Node/InstructionNode.h"
#include "Node/ConditionalStructNode.h"

using namespace Nodable;

enum TraversalFlag_ {
    TraversalFlag_None             = 0,
    TraversalFlag_FollowInputs     = 1 << 0,
    TraversalFlag_FollowOutputs    = 1 << 1,
    TraversalFlag_FollowChildren   = 1 << 2,
    TraversalFlag_FollowParent     = 1 << 3,
    TraversalFlag_FollowNotDirty     = 1 << 4,
};

Result GraphTraversal::update(Node* _rootNode)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "Update %s \n", _rootNode->getLabel() );
    auto result = traverseRec(_rootNode, TraversalFlag_FollowInputs | TraversalFlag_FollowChildren | TraversalFlag_FollowNotDirty);
    for(Node* eachNode : stats.traversed )
    {
        if ( eachNode->isDirty() )
        {
            eachNode->eval();
            eachNode->update();
            eachNode->setDirty(false);
            stats.changed.push_back(eachNode);
        }
    }


    LOG_VERBOSE("GraphTraversal", "GraphTraversal::Update done.\n");
    return result;
}

Result GraphTraversal::setDirty(Node* _rootNode)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::setDirty %s \n", _rootNode->getLabel() );
    auto result = traverseRec(_rootNode, TraversalFlag_FollowOutputs);
    for(Node* eachNode : stats.traversed )
        eachNode->update();
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::setDirty done.\n");
    return result;
}

//Result GraphTraversal::update(ScopedCodeBlockNode *_scope)
//{
//    initialize();
//    LOG_VERBOSE("GraphTraversal", "GraphTraversal::update %s \n", _scope->getLabel() );
//    auto result = updateRecursively(_scope);
//    LOG_VERBOSE("GraphTraversal", "GraphTraversal::update done.\n");
//    return result;
//}

Result GraphTraversal::traverseForEval(Node* _node)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverseForEval %s \n", _node->getLabel() );
    auto result = traverseRec(_node, TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty );
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverseForEval done.\n");
    return result;
}

Result GraphTraversal::traverse(Node *_node, TraversalFlag _flags)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverse %s \n", _node->getLabel() );
    auto result = traverseRec(_node, _flags);
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverse done.\n");
    return result;
}

Result GraphTraversal::traverseRec(Node* _node, TraversalFlag _flags)
{

    if( !stats.hasBeenTraversed(_node) )
    {
        if ( _node->isDirty() || (_flags & TraversalFlag_FollowNotDirty ) )
        {
            if ( _flags & TraversalFlag_FollowInputs )
            {
                for (auto eachInput : _node->getInputs())
                {
                    if ( traverseRec(eachInput, _flags) == Result::Failure)
                        return Result::Failure;
                }
            }

            stats.traversed.push_back(_node);

            if ( _flags & TraversalFlag_FollowChildren )
            {
                for (auto eachChild :  _node->getChildren())
                {
                    if ( traverseRec(eachChild, _flags) == Result::Failure)
                        return Result::Failure;
                }
            }

            if ( _flags & TraversalFlag_FollowParent )
            {
                if (auto parent = _node->getParent())
                {
                    if ( traverseRec(parent, _flags) == Result::Failure)
                        return Result::Failure;
                }
            }
        }

        return Result::Success;

    }
    LOG_WARNING("GraphTraversal", "Unable to update Node %s, cycle detected.\n", _node->getLabel() );
    return Result::Failure;
}

Node* GraphTraversal::getNextInstrToEval(Node *_node)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "getNextInstrToEval( %s )...\n", _node->getLabel() );
    auto result = getNextInstrToEvalRec(_node);
    if (result)
        LOG_VERBOSE("GraphTraversal", "%s's next is %s\n", _node->getLabel(), result->getLabel());

    return result;
}

void GraphTraversal::initialize()
{
    this->stats = {};
}


//Result GraphTraversal::setDirtyRecursively(Node* _node) {
//
//    Result result;
//
//    LOG_VERBOSE("GraphTraversal", "GraphTraversal::SetDirtyEx\n");
//
//    if( !stats.hasBeenTraversed(_node) )
//    {
//        stats.traversed.push_back(_node);
//
//        _node->setDirty();
//
//        for (auto eachOutput : _node->getOutputs() )
//        {
//            auto r = setDirtyRecursively(eachOutput);
//            if( r == Result::Failure )
//                return Result::Failure;
//        };
//
//        for (auto& eachChild : _node->getChildren() )
//        {
//            auto r = setDirtyRecursively(eachChild);
//            if( r == Result::Failure )
//                return Result::Failure;
//        }
//
//        if( auto parent = _node->getParent())
//        {
//            auto r = setDirtyRecursively(parent);
//            if( r == Result::Failure )
//                return Result::Failure;
//        }
//
//        result = Result::Success;
//    } else {
//        result = Result::Failure;
//    }
//
//    return result;
//}

//Result GraphTraversal::traverseForEvalRecursively(Node* _node) {
//
//    Result result;
//    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverseForEvalRecursively %s\n", _node->getLabel());
//
//    if( !stats.hasBeenTraversed(_node) )
//    {
//        // first we do a recursive call
//        for (auto eachInput : _node->getInputs())
//        {
//            if( traverseForEvalRecursively(eachInput) == Result::Failure )
//                return Result::Failure;
//        }
//
//        // then we push node
//        stats.traversed.push_back(_node);
//        result = Result::Success;
//    } else {
//        result = Result::Failure;
//        LOG_WARNING("GraphTraversal", "Unable to traverseForEvalRecursively Node %s, cycle detected.\n", _node->getLabel() );
//    }
//
//    return result;
//}
//
//Result GraphTraversal::updateRecursively(Node* _node) {
//
//    Result result;
//    LOG_VERBOSE("GraphTraversal", "GraphTraversal::UpdateEx %s\n", _node->getLabel());
//
//    if( !stats.hasBeenTraversed(_node) && _node->isDirty() )
//    {
//        stats.traversed.push_back(_node);
//
//        for (auto eachInput : _node->getInputs())
//        {
//            if ( updateRecursively(eachInput) == Result::Failure)
//                return Result::Failure;
//        }
//
//        _node->update();
//
//        for (auto eachChild :  _node->getChildren())
//        {
//            if ( updateRecursively(eachChild) == Result::Failure)
//                return Result::Failure;
//        }
//
//        result = Result::Success;
//
//    } else {
//        result = Result::Failure;
//        LOG_WARNING("GraphTraversal", "Unable to update Node %s, cycle detected.\n", _node->getLabel() );
//    }
//
//    return result;
//}

void GraphTraversal::logStats()
{
    LOG_MESSAGE("GraphTraversal", "traversed %i node(s).\n", (int)stats.traversed.size());
}
//
//bool GraphTraversal::hasAChildDirty(Node *_node)
//{
//    initialize();
//    return hasAChildDirtyRec(_node);
//}
//
//bool GraphTraversal::hasAChildDirtyRec(Node *_node)
//{
//    bool result = false;
//    LOG_VERBOSE("GraphTraversal", "GraphTraversal::UpdateEx\n");
//
//    if( !stats.hasBeenTraversed(_node) )
//    {
//        stats.traversed.push_back(_node);
//        result = _node->isDirty();
//
//        if( !result && !_node->getChildren().empty())
//        {
//            for (auto& eachChild : _node->getChildren() )
//            {
//                result |= hasAChildDirtyRec(eachChild);
//            }
//        }
//
//    } else {
//        LOG_WARNING("GraphTraversal", "Unable to update Node %s, cycle detected.\n", _node->getLabel() );
//    }
//
//    return result;
//}

bool Stats::hasBeenTraversed(const Node* _node) const
{
    return  std::find( traversed.cbegin(), traversed.cend(), _node ) != traversed.cend();
}
bool Stats::hasBeenChanged(const Node* _node) const
{
    return  std::find( changed.cbegin(), changed.cend(), _node ) != changed.cend();
}

Node* GraphTraversal::getNextInstrToEvalRec(Node* _node)
{
    NODABLE_ASSERT(!stats.hasBeenTraversed(_node));
    stats.traversed.push_back(_node);

    /*
     * Get the next Node from an execution point of view.
     */
    Node* result  = nullptr;
    auto clss     = _node->getClass();
    auto children = _node->getChildren();

    if ( clss == mirror::GetClass<ConditionalStructNode>())
    {
       /*
        * Get the branch depending on condition
        */
       auto next = _node->getNext(); // is virtual
       if ( !stats.hasBeenTraversed(next) )
           result = next;
    }
    else if ( !children.empty() )//if ( clss->isChildOf( mirror::GetClass<AbstractCodeBlockNode>() ) )
    {
        /*
         * Get the first not already traversed child
         */
        auto notAlreadyTraversed = [&](auto each )-> bool {
            return !stats.hasBeenTraversed(each);
        };
        auto found = std::find_if(children.begin(), children.end(), notAlreadyTraversed);
        if ( found != children.end())
            result = *found;
    }

    if ( result == nullptr )
    {
        if ( auto parent = _node->getParent() )
        {
            // set previous children traversed
            auto it = parent->getChildren().begin();
            while( *it != _node)
            {
                stats.traversed.push_back(*it);
                it++;
            }

            result = getNextInstrToEvalRec(parent);
        }
    }
    else
    {
        mirror::Class* resultClass = result->getClass();
        if (resultClass == mirror::GetClass<ScopedCodeBlockNode>() || resultClass == mirror::GetClass<CodeBlockNode>())
        {
            result = getNextInstrToEvalRec(result);
        }
    }
    stats.traversed.push_back(result);

    return result;
}
