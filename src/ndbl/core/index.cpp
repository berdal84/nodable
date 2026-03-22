#include "index.h"
#include "tools/core/index.h"
#include "ASTFunctionCall.h"
#include "ASTLiteral.h"
#include "ASTVariable.h"

using namespace ndbl;

void ndbl::init_reflection()
{
    tools::init_reflection();
    
    DEFINE_REFLECT(ASTFunctionCall).extends<ASTNode>();
    DEFINE_REFLECT(ASTLiteral).extends<ASTNode>();
    DEFINE_REFLECT(ASTNode);
    DEFINE_REFLECT(ASTScope).extends<tools::Component<ASTNode>>();
    DEFINE_REFLECT(ASTVariable).extends<ASTNode>();
} 