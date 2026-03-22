#include "ndbl/gui/index.h"

// unity build
#include "ndbl/core/ASTNodeSlot.specs.cpp"
#include "ndbl/core/ASTToken.specs.cpp"
#include "ndbl/core/Graph.specs.cpp"
#include "ndbl/core/language/Nodlang.basics.specs.cpp"
#include "ndbl/core/language/Nodlang.parse_and_serialize.specs.cpp"
#include "ndbl/core/language/Nodlang.parse_function_call.specs.cpp"
#include "ndbl/core/language/Nodlang.parse_token.specs.cpp"
#include "ndbl/core/language/Nodlang.tokenize.specs.cpp"
// unity build (end)

#include <gtest/gtest.h>

int main(int argc, char **argv)
{
    ndbl::init_reflection_with_gui();
    tools::set_log_verbosity(tools::Verbosity_Message);
    tools::TypeRegister::log_statistics();

    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}