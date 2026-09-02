#include "index.h"
#include "bdc/Allocators.hpp"
#include "tools/core/reflection/Initializer.h"
#include "ndbl/core/index.h"
#include "ndbl/core/Node.h"
#include "ndbl/gui/Graph_View.h"

void ndbl::init_with_gui()
{
    bdc::memory_manager_init(1024 * 1024 * 10);
    ndbl::init_reflection();
    
    DEFINE_REFLECT(ndbl::Node_View);
    DEFINE_REFLECT(ndbl::Graph_View);
}

void ndbl::shutdown_with_gui()
{
    bdc::memory_manager_shutdown();
} 