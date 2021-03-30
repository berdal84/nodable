#include "ConditionalStructNode.h"

using namespace Nodable;

ConditionalStructNode::ConditionalStructNode()
    :
    token_if(nullptr),
    token_else(nullptr)
{
    this->add("condition", Visibility::Always, Type::Boolean, Way::Way_In);
}
