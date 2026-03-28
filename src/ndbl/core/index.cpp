#include "index.h"
#include "tools/core/index.h"
#include "ASTNode.h"

void ndbl::init_reflection()
{
    tools::init_reflection();
    
    DEFINE_REFLECT(ASTNode);
    DEFINE_REFLECT(ASTScope).extends<tools::Component<ASTNode>>();
} 