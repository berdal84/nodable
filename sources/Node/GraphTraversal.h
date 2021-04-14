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

        /** Set dirty a Node and all its output connected Nodes recursively */
        Result setDirty(Node* _rootNode);

        /** Update a Node after its input connected Nodes (only if dirty) */
        Result update(Node* _rootNode);

//        /** Evaluate a given scope */
//        Result update(ScopedCodeBlockNode *_scope);

        /** Traverse following nodes like if we were evaluating */
        Result traverseForEval(Node* _rootNode);

        Result traverse(Node*, TraversalFlag);

        /** get statistics, usually needed after a traversal */
        [[nodiscard]] const Stats& getStats() const { return stats; }

        void logStats();

//        bool hasAChildDirty(Node *_node);

        Node* getNext(Node *_node);
    private:
        /** initialize this node traversal to make it like a new instance */
        void initialize();

//        Result setDirtyRecursively(Node* _node);
//        Result updateRecursively(Node* _node);
//        Result traverseForEvalRecursively(Node* _node);
        Node* getNextRec(Node *_node);
        Result traverseRec(Node* _node, TraversalFlag _flags);
//        bool hasAChildDirtyRec(Node* _node);
        Stats stats;
    };
}