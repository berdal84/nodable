#pragma once
#include "ndbl/core/ASTNode.h"
#include "tools/gui/geometry/SpatialNode.h"

namespace ndbl
{
    class SpatialNodeComponent :  public tools::ComponentFor<ASTNode>
    {
    public:
        tools::SpatialNode* data() { return &_data; }
    private:
        tools::SpatialNode _data;
    };
}