#pragma once
#include "Code.h"
#include "core/Graph.h"
#include "fw/core/types.h"

namespace ndbl
{
    // forward declarations
    class IfNode;
    class ForLoopNode;
    class WhileLoopNode;
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
        bool is_syntax_tree_valid(const Graph*);                                  // Check if syntax tree has a valid syntax (declared variables and functions).
        void compile_node( const Node* _node);                              // Compile a node recursively, result depends on node type.
        void compile_output_slot( const Slot *slot );                             // Compile from a Slot recursively (if targets a "this" property will compile "this" node, if not will compile input property recursively).
        void compile_scope(const Scope*, bool _insert_fake_return = false);       // Compile a scope recursively, optionally insert a fake return statement (lack of return" keyword").
        void compile_instruction_as_condition( const Node* _instr_node );         // Compile an instruction as a condition.
        void compile_for_loop(const ForLoopNode*);                                // Compile a "for loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile_while_loop(const WhileLoopNode*);                            // Compile a "while loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile_conditional_struct(const IfNode*);                           // Compile an "if/else" recursively.
        void compile_iterative_structure( const IConditional* _conditional, const char* _comment );

        Code* m_temp_code;  // Store the code being compiled, is released when compilation ends.
    };
} // namespace assembly
} // namespace nodable
