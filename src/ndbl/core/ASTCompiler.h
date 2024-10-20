#pragma once
#include "tools/core/types.h"
#include "ASTGraph.h"
#include "Code.h"

namespace ndbl
{
    // forward declarations
    class ASTConditionalNode;
    class ASTForLoopNode;
    class ASTWhileLoopNode;
    class InstructionNode;
    class ASTNode;
    class Property;
    class ASTScope;

    /**
     * @class Class to compile a syntax tree (Graph) to a simple instruction list (Assembly::Code)
     */
    class ASTCompiler
    {
    public:
        ASTCompiler()= default;
        const Code* compile_ast(const ASTGraph*); // Compile the entire abstract syntax tree, and return a code that Interpreter can load.
    private:
        bool is_syntax_tree_valid(const ASTGraph*);                               // Check if syntax tree has a valid syntax (declared variables and functions).
        void compile_node( const ASTNode*);                                       // Compile a node recursively, result depends on node type.
        void compile_input_slot( const Slot& );                                   // Compile from a Slot recursively (slot must be an OUTPUT).
        void compile_output_slot( const Slot& );                                  // Compile from a Slot recursively (slot must be an INPUT).
        void compile_scope(const ASTScope*, bool _insert_fake_return = false);    // Compile a scope recursively, optionally insert a fake return statement (lack of return" keyword").
        void compile_instruction_as_condition(const ASTNode* );                   // Compile an instruction as a condition (stores result in a register)
        void compile_for_loop(const ASTForLoopNode*);                             // Compile a "for loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile_while_loop(const ASTWhileLoopNode*);                         // Compile a "while loop" recursively (initial, condition, iterative instructions and inner scope).
        void compile_conditional_struct(const ASTConditionalNode*);               // Compile an "if/else" recursively.

        Code* m_temp_code;  // Store the code being compiled, is released when compilation ends.
    };
} // namespace ndbl
