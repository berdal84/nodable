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
    tools::Pool::init();
    tools::init_task_manager();
}

void NodableHeadless::shutdown()
{
    graph.clear();
    tools::Pool::shutdown();
    tools::shutdown_task_manager();
}
