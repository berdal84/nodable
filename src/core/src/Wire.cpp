#include <nodable/core/Wire.h>

using namespace ndbl;

void Wire::sanitize()
{
    NODABLE_ASSERT(members.src != members.dst);
}
