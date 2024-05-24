#include "NodableHeadless.h"
#include "tools/core/async.h"

using namespace ndbl;

NodableHeadless::NodableHeadless()
: graph(&factory)
, compiler()
{
}

void NodableHeadless::init()
{
    tools::init_pool_manager();
    tools::init_task_manager();
}

void NodableHeadless::shutdown()
{
    graph.clear();
    tools::shutdown_pool_manager();
    tools::shutdown_task_manager();
}
