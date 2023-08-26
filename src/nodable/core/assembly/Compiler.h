#pragma once
#include "fw/core/types.h"
#include "Code.h"

namespace ndbl
{
    // forward declarations
    class ConditionalStructNode;
    class ForLoopNode;
    class WhileLoopNode;
    class Graph;
    class InstructionNode;
    class Node;
    class Property;
    class Scope;

namespace assembly
{
    /**
     * @class Class to compile a syntax tree (Graph) to a simple instruction list (Assembly::Code)
     */
    class Compiler
    {
    public:
        Compiler()= default;
        const Code* compile_syntax_tree(const Graph *_graph);        // Compile the full syntax tree (a.k.a. graph) and return dynamically allocated code that VirtualMachine can load.
    private:
        bool is_syntax_tree_valid(const Graph*);                              // Check if syntax tree has a valid syntax (declared variables and functions).
        void compile(ID<const Node> _node);                                                // Compile a node recursively, result depends on node type.
        void compile(const Property *);                                           // Compile a property recursively (if targets a "this" property will compile "this" node, if not will compile input property recursively).
        void compile(const Scope*, bool _insert_fake_return = false);             // Compile a scope recursively, optionally insert a fake return statement (lack of return" keyword").
        void compile(const InstructionNode*);                                     // Compile an instruction (will compile its root recursively).
        void compile_as_condition(const InstructionNode *_instr_node);            // Compile an instruction as a condition.
        void compile(const ForLoopNode*);                                         // Compile a "for loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile(const WhileLoopNode*);                                       // Compile a "while loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile(const ConditionalStructNode*);                               // Compile an "if/else" recursively.

        Code* m_temp_code;  // Store the code being compiled, is released when compilation ends.
    };
} // namespace assembly
} // namespace nodable
