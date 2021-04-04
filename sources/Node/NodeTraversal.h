#pragma once

#include "Nodable.h"
#include "Node.h"
#include <vector>

namespace Nodable {

    enum class Result {
        Success,
        Failure
    };

    // forward declarations
    class ScopedCodeBlockNode;

    /**
     * Structure to store some statistics about a traversal
     */
    struct Stats {
        std::vector<const Node*> traversed;
        /** Return true if _node has been traversed, false otherwise */
        bool hasBeenTraversed(const Node* _node) const;
    };

    /**
     * @brief 
     * The intend of this class is to traverse Node hierarchy and do some operations on them.
     * 
     * TODO: create a generic method: NodeTraversal::Traverse(_rootNode, _operation, _branchStrategy ) ?
     * 
     */
    class NodeTraversal {
        friend class Node;
    public:
        NodeTraversal() = default;
        ~NodeTraversal() = default;

        /** Set dirty a Node and all its output connected Nodes recursively */
        Result setDirty(Node* _rootNode);

        /** Update a Node after its input connected Nodes (only if dirty) */
        Result update(Node* _rootNode);

        /** Evaluate a given scope */
        Result update(ScopedCodeBlockNode *_scope);

        /** get statistics, usually needed after a traversal */
        [[nodiscard]] const Stats& getStats() const { return stats; }

        void logStats();

        bool hasAChildDirty(const Node *_node);

        Node* getNext(Node *_node);
    private:
        /** initialize this node traversal to make it like a new instance */
        void initialize();

        Node* getNextRec(Node *_node);
        Result setDirtyRecursively(Node* _node);
        Result updateRecursively(Node* _node);
        bool hasAChildDirtyRec(const Node* _node);
        Stats stats;
    };
}