#include <nodable/GraphTraversal.h>

#include <algorithm>

#include <nodable/Wire.h>
#include <nodable/Log.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/VariableNode.h>

using namespace Nodable;

Result GraphTraversal::TraverseAndSetDirty(Node *_rootNode)
{
    GraphTraversal traversal;
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::TraverseAndSetDirty %s \n", _rootNode->getLabel() )
    auto result = traversal.traverseRec(_rootNode, TraversalFlag_FollowOutputs);
    for(Node* eachNode : traversal.m_stats.m_traversed )
        eachNode->setDirty();
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::TraverseAndSetDirty done.\n")
    return result;
}

Result GraphTraversal::traverse(Node *_node, TraversalFlag _flags)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverse %s \n", _node->getLabel() )
    auto result = traverseRec(_node, _flags);

    if ( _flags & TraversalFlag_ReverseResult )
    {
        std::reverse(m_stats.m_traversed.begin(), m_stats.m_traversed.end());
        std::reverse(m_stats.m_changed.begin(), m_stats.m_changed.end());
    }
    LOG_VERBOSE("GraphTraversal", "GraphTraversal::traverse done.\n")
    return result;
}

Result GraphTraversal::traverseRec(Node* _node, TraversalFlag _flags)
{
    if( !m_stats.hasBeenTraversed(_node) )
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

            m_stats.m_traversed.push_back(_node);

            if ( _flags & TraversalFlag_FollowChildren )
            {
                for (auto eachChild :  _node->get_children())
                {
                    if ( traverseRec(eachChild, _flags) == Result::Failure)
                        return Result::Failure;
                }
            }

            if ( _flags & TraversalFlag_FollowParent )
            {
                if (auto parent = _node->get_parent())
                {
                    if ( traverseRec(parent, _flags) == Result::Failure)
                        return Result::Failure;
                }
            }
        }

        return Result::Success;

    }
    else if ( _flags & TraversalFlag_AvoidCycles )
    {
        NODABLE_ASSERT(_node->getClass() == VariableNode::GetClass());
        return Result::Success;
    }

    LOG_ERROR("GraphTraversal", "Unable to update Node %s, cycle detected (consider using TraversalFlag_AvoidCycles).\n", _node->getLabel() )
    return Result::Failure;

}

Node* GraphTraversal::getNextInstrToEval(Node *_node)
{
    initialize();
    LOG_VERBOSE("GraphTraversal", "getNextInstrToEval( %s )...\n", _node->getLabel() )
    auto result = getNextInstrToEvalRec(_node);
    if (result)
    {
        LOG_VERBOSE("GraphTraversal", "%s's next is %s\n", _node->getLabel(), result->getLabel())
    }

    return result;
}

void GraphTraversal::initialize()
{
    this->m_stats = {};
}

void GraphTraversal::logStats()
{
    LOG_MESSAGE("GraphTraversal", "traversed %i node(s).\n", (int)m_stats.m_traversed.size())
}

bool Stats::hasBeenTraversed(const Node* _node) const
{
    return std::find(m_traversed.cbegin(), m_traversed.cend(), _node ) != m_traversed.cend();
}
bool Stats::hasBeenChanged(const Node* _node) const
{
    return std::find(m_changed.cbegin(), m_changed.cend(), _node ) != m_changed.cend();
}

Node* GraphTraversal::getNextInstrToEvalRec(Node* _node)
{
    NODABLE_ASSERT(!m_stats.hasBeenTraversed(_node));
    m_stats.m_traversed.push_back(_node);

    /*
     * Get the next Node from an execution point of view.
     */
    Node* result  = nullptr;
    auto children = _node->get_children();

    if ( auto condStructNode = _node->as<ConditionalStructNode>() )
    {
       /*
        * Get the branch depending on condition
        */
       auto next = *condStructNode->getCondition() ? condStructNode->getBranchTrue() : condStructNode->getBranchFalse();
       if ( !m_stats.hasBeenTraversed(next) )
           result = next;
    }
    else if ( !children.empty() )//if ( clss->isChildOf( mirror::GetClass<AbstractCodeBlockNode>() ) )
    {
        /*
         * Get the first not already traversed child
         */
        auto notAlreadyTraversed = [&](auto each )-> bool {
            return !m_stats.hasBeenTraversed(each);
        };
        auto found = std::find_if(children.begin(), children.end(), notAlreadyTraversed);
        if ( found != children.end())
            result = *found;
    }

    if ( result == nullptr )
    {
        if ( auto parent = _node->get_parent() )
        {
            // set previous children traversed
            auto it = parent->get_children().begin();
            while( *it != _node)
            {
                m_stats.m_traversed.push_back(*it);
                it++;
            }

            result = getNextInstrToEvalRec(parent);
        }
    }
    else
    {
        Reflect::Class* resultClass = result->getClass();
        if (resultClass == ScopedCodeBlockNode::GetClass() || resultClass == CodeBlockNode::GetClass())
        {
            result = getNextInstrToEvalRec(result);
        }
    }
    m_stats.m_traversed.push_back(result);

    return result;
}
