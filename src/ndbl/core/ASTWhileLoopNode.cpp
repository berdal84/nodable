#include "ASTWhileLoopNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<ASTWhileLoopNode>("WhileLoopNode").extends<ASTNode>();
}

void ASTWhileLoopNode::init(const std::string &_name)
{
    ASTNode::init(ASTNodeType_BLOCK_WHILE_LOOP, _name);
    m_wrapped_conditional.init(this);
}
