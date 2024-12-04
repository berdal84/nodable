#pragma once
#include "ndbl/core/ASTNode.h"
#include "tools/gui/ViewState.h"

namespace ndbl
{
    class ViewStateComponent : public tools::ComponentFor<ASTNode>
    {
    public:
        tools::ViewState* data() { return &_data; }
    private:
        tools::ViewState _data;
    };
}