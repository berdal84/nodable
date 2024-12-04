#pragma once
#include "ndbl/core/ASTNode.h"
#include "tools/gui/geometry/BoxShape2D.h"

namespace ndbl
{
    class BoxShapeComponent :  public tools::ComponentFor<ASTNode>
    {
    public:
         BoxShapeComponent(const tools::BoxShape2D& data): ComponentFor<ASTNode>("Shape"), _data(data) {}
        void               set_data(const tools::BoxShape2D& data) { _data = data; }
        tools::BoxShape2D* data() { return &_data; }
    private:
        tools::BoxShape2D _data;
    };
}