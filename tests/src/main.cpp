#include <gtest/gtest.h>
#include <nodable/Log.h>
#include <mirror.h>
#include <nodable/Node.h>
#include <nodable/GraphNode.h>
#include <nodable/VariableNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/ComputeUnaryOperation.h>
#include <nodable/ComputeBinaryOperation.h>

using namespace Nodable;
using Verbosity = Log::Verbosity;

MIRROR_INITIALIZER
MIRROR_CLASS_DEFINITION(Node)
MIRROR_CLASS_DEFINITION(GraphNode)
MIRROR_CLASS_DEFINITION(VariableNode)
MIRROR_CLASS_DEFINITION(LiteralNode)
MIRROR_CLASS_DEFINITION(InstructionNode)
MIRROR_CLASS_DEFINITION(AbstractCodeBlockNode)
MIRROR_CLASS_DEFINITION(CodeBlockNode)
MIRROR_CLASS_DEFINITION(ScopedCodeBlockNode)
MIRROR_CLASS_DEFINITION(ConditionalStructNode)
MIRROR_CLASS_DEFINITION(Component)
MIRROR_CLASS_DEFINITION(ComputeBase)
MIRROR_CLASS_DEFINITION(ComputeFunction)
MIRROR_CLASS_DEFINITION(ComputeUnaryOperation)
MIRROR_CLASS_DEFINITION(ComputeBinaryOperation)

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
//    Log::SetVerbosityLevel("Parser", Verbosity::Verbose);
//    Log::SetVerbosityLevel("GraphNode", Verbosity::Verbose);
    return RUN_ALL_TESTS();
}