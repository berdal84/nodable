#include "NodableHeadless.h"
#include "VirtualMachine.h"
#include "ndbl/core/language/Nodlang.h"
#include "tools/core/async.h"

using namespace ndbl;

NodableHeadless::NodableHeadless()
: graph( nullptr)
, compiler()
{
}

void NodableHeadless::init()
{
    tools::init_pool_manager();
    tools::init_task_manager();
    ndbl::init_language();
    ndbl::init_node_factory();
    ndbl::init_virtual_machine();
    graph = new Graph(get_node_factory());
}

void NodableHeadless::shutdown()
{
    delete graph;
    tools::shutdown_pool_manager();
    tools::shutdown_task_manager();
    ndbl::shutdown_language();
    ndbl::shutdown_node_factory();
    ndbl::shutdown_virtual_machine();
}
