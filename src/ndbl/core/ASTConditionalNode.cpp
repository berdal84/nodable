#include "ASTUtils.h"
#include "ASTConditionalNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ASTConditionalNode>("IfNode").extends<ASTNode>();
}


void ASTConditionalNode::init(const std::string&_name)
{
    ASTNode::init(ASTNodeType_BLOCK_CONDITION, _name);
    m_wrapped_conditional.init(this);
}
// required to link static code above