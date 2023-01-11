#include <nodable/core/Wire.h>

using namespace ndbl;

void Wire::sanitize()
{
    NDBL_ASSERT(members.src != members.dst);
}
