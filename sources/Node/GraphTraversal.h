#pragma once

#include "Nodable.h"
#include "Node.h"
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
    };

    // forward declarations
    class ScopedCodeBlockNode;

    /**
     * Structure to store some statistics about a traversal
     */
    struct Stats {
        std::vector<Node*> traversed;
        std::vector<Node*> changed;
        /** Return true if _node has been traversed, false otherwise */
        bool hasBeenTraversed(const Node* _node) const;
        bool hasBeenChanged(const Node* _node) const;
    };

    /**
     * @brief 
     * The intend of this class is to traverse Node hierarchy and do some operations on them.
     * 
     * TODO: create a generic method: GraphTraversal::Traverse(_rootNode, _operation, _branchStrategy ) ?
     * 
     */
    class GraphTraversal {
        friend class Node;
    public:
        GraphTraversal() = default;
        ~GraphTraversal() = default;
        Result setDirty(Node* _rootNode);
        Result update(Node* _rootNode);
        Result traverse(Node*, TraversalFlag);
        [[nodiscard]] inline const Stats& getStats() const { return stats; }
        void logStats();
        Node* getNextInstrToEval(Node *_node);
    private:
        void initialize();
        Node* getNextInstrToEvalRec(Node *_node);
        Result traverseRec(Node* _node, TraversalFlag _flags);
        Stats stats;
    };
}