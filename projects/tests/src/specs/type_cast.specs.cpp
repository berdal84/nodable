#include <gtest/gtest.h>
#include "../fixtures/core.h"
#include "fw/core/log.h"

using namespace ndbl;
typedef ::testing::Core Eval_With_Type_Cast;

TEST_F(Eval_With_Type_Cast, allowed_i16_to_double )
{
    EXPECT_EQ(eval<double>("double d = 15"), 15.0);
}

TEST_F(Eval_With_Type_Cast, not_allowed_double_to_i16 )
{
    EXPECT_ANY_THROW(eval<i16_t>("int i = 15.5"));
}

