#pragma once

#include "bdc/Allocators.hpp"
#include "ndbl/core/reflection/index.h"
#include <gtest/gtest.h>

namespace testing
{
using namespace ndbl;
using namespace bdc;

class Basic_Test : public Test
{
public:
    void SetUp() override
    {
        reflection_init();
        memory_manager_init();
    }

    void TearDown() override
    {
        memory_manager_shutdown();
        reflection_shutdown();
    }
};
}
