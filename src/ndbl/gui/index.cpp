#include "index.h"
#include "tools/core/Component.h"
#include "tools/core/reflection/Initializer.h"
#include "ndbl/core/index.h"
#include "ndbl/core/Node.h"
#include "ndbl/gui/Graph_View.h"

void ndbl::init_reflection_with_gui()
{
    ndbl::init_reflection();
    
    DEFINE_REFLECT(ndbl::Node_View).extends<tools::Component<Node>>();
    DEFINE_REFLECT(ndbl::Graph_View).extends<tools::Component<Graph>>();
} 