#pragma once

#include <nodable/Nodable.h>
#include <nodable/Node.h>
#include <vector>

namespace Nodable {

    enum class Result {
        Success,
        Failure
    };

    typedef int TraversalFlag; // -> enum TraversalFlag_
    enum TraversalFlag_ {
        TraversalFlag_None             = 0,
        TraversalFlag_FollowInputs     = 1 << 0,
        TraversalFlag_FollowOutputs    = 1 << 1,
        TraversalFlag_FollowChildren   = 1 << 2,
        TraversalFlag_FollowParent     = 1 << 3,
        TraversalFlag_FollowNotDirty   = 1 << 4,
        TraversalFlag_ReverseResult    = 1 << 5,
        TraversalFlag_AvoidCycles      = 1 << 6
    };

    // forward declarations
    class ScopeNode;

    /**
     * Structure to store some statistics about a traversal
     */
    struct Stats {
        std::vector<Node*> m_traversed;
        std::vector<Node*> m_changed;
        /** Return true if _node has been traversed, false otherwise */
        bool hasBeenTraversed(const Node* _node) const;
        bool hasBeenChanged(const Node* _node) const;
    };

    /**
     * @brief 
     * The intend of this class is to traverse Node hierarchy and do some operations on them.
     */
    class GraphTraversal {
        friend class Node;
    public:
        GraphTraversal() = default;
        ~GraphTraversal() = default;
                      /** generic traverse method */
                      Result              traverse(Node*, TraversalFlag);
                      void                logStats();
        [[nodiscard]] Node*               getNextInstrToEval(Node *_node);
        [[nodiscard]] inline const Stats& getStats() const { return m_stats; }

                      static Result       TraverseAndSetDirty(Node* _rootNode);

    private:
        void    initialize();
        Node*   getNextInstrToEvalRec(Node *_node);
        Result  traverseRec(Node* _node, TraversalFlag _flags);

        Stats m_stats;
    };
}