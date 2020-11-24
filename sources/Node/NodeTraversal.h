#pragma once

#include "Nodable.h"
#include "Node.h"

#include <vector>

namespace Nodable {

    enum class Result {
        Success,
        Failure
    };

    /**
     * @brief 
     * The intend of this class is to traverse Node hierarchy and evaluates them.
     * 
     * TODO: create a generic method: NodeTraversal::Traverse(_rootNode, _operation, _branchStrategy )
     * 
     */
    class NodeTraversal {
        friend class Node;
    public:

        /* Set dirty a Node and all its descendants */
        static Result SetDirty(Node* _rootNode);

        /* Update a Node with its ascendants (only if needed) */
        static Result Update(Node* _rootNode);

    private:
        static Result SetDirtyRecursively(Node* _node, std::vector<Node*>& _traversed);
        static Result UpdateRecursively(Node* _node, std::vector<Node*>& _traversed);

    };
}