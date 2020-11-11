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

        /**
         * Set dirty a Node and all its descendants
         * Detects cycles.
         */
        static Result SetDirty(const std::shared_ptr<Node>&  _rootNode);

        /**
         * Update a Node with its ascendants (only if needed)
         * Detect cycles.
         */
        static Result Update(const std::shared_ptr<Node>& _rootNode);

    private:
        static Result SetDirtyRecursively(const std::shared_ptr<Node>& _node, std::vector<std::shared_ptr<Node>>& _traversed);
        static Result UpdateRecursively(const std::shared_ptr<Node>& _node, std::vector<std::shared_ptr<Node>>& _traversed);
    };
}