#include "index.h"
#include "tools/core/index.h"
#include "Node.h"

void ndbl::init_reflection()
{
    tools::init_reflection();
    
    DEFINE_REFLECT(Node);
    DEFINE_REFLECT(Scope).extends<tools::Component<Node>>();
} 