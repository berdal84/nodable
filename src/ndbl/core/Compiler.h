#pragma once
#include "tools/core/types.h"
#include "Graph.h"
#include "Code.h"

namespace ndbl
{
    // forward declarations
    class ASTIf;
    class ASTForLoop;
    class ASTWhileLoop;
    class InstructionNode;
    class ASTNode;
    class ASTNodeProperty;
    class ASTScope;

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
        void compile_node( const ASTNode*);                                          // Compile a node recursively, result depends on node type.
        void compile_input_slot(const ASTNodeSlot*);                                     // Compile from a Slot recursively (slot must be an OUTPUT).
        void compile_output_slot(const ASTNodeSlot*);                                    // Compile from a Slot recursively (slot must be an INPUT).
        void compile_inner_scope(const ASTNode*, bool _insert_fake_return = false);  // Compile a scope recursively, optionally insert a fake return statement (lack of return" keyword").
        void compile_instruction_as_condition(const ASTNode* );                      // Compile an instruction as a condition (stores result in a register)
        void compile_for_loop(const ASTForLoop*);                                // Compile a "for loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile_while_loop(const ASTWhileLoop*);                            // Compile a "while loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile_conditional_struct(const ASTIf*);                           // Compile an "if/else" recursively.

        Code* m_temp_code;  // Store the code being compiled, is released when compilation ends.
    };
} // namespace ndbl
