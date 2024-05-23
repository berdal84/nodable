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
    tools::async::init();
}

void NodableHeadless::shutdown()
{
    graph.clear();
    tools::Pool::shutdown();
    tools::async::shutdown();
}
