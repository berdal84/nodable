#include <gtest/gtest.h>
#include <cmath> // for pow()

#include <nodable/core/Log.h>
#include "../nodable_fixture.h"

using namespace Nodable;

TEST_F(nodable_fixture, allowed_i16_to_double )
{
    EXPECT_EQ(eval<double>("double d = 15"), 15.0);
}

TEST_F(nodable_fixture, not_allowed_double_to_i16 )
{
    EXPECT_ANY_THROW(eval<i16_t>("int i = 15.5"));
}

