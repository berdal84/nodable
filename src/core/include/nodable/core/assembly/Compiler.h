#pragma once

#include <memory>

#include <nodable/core/types.h>
#include <nodable/core/assembly/Code.h>

namespace Nodable
{
    // forward declarations
    class GraphNode;
    class Node;
    class Scope;
    class InstructionNode;
    class ForLoopNode;
    class ConditionalStructNode;

namespace assembly
{
    /**
     * @brief Convert a syntax tree (GraphNode) to a simple instruction list (Assembly::Code)
     */
    class Compiler
    {
    public:
        Compiler()= default;
        std::unique_ptr<const Code> compile_syntax_tree(const GraphNode*);
    private:
        bool is_syntax_tree_valid(const GraphNode*);
        void compile(const Node*);
        void compile(const Member*);
        void compile(const Scope*, bool _insert_fake_return = false);
        void compile(const InstructionNode*);
        void compile(const ForLoopNode*);
        void compile(const ConditionalStructNode*);
        void compile_as_condition(const InstructionNode *_instr_node);

        std::unique_ptr<Code> m_temp_code;
    };
} // namespace Asm
} // namespace Nodable
