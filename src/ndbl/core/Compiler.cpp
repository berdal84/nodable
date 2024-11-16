#include "Compiler.h"

#include <exception>
#include <iostream>

#include "tools/core/assertions.h"
#include "tools/core/log.h"
#include "tools/core/math.h"
#include "tools/core/string.h"
#include "tools/core/memory/memory.h"

#include "ndbl/core/ForLoopNode.h"
#include "ndbl/core/Graph.h"
#include "ndbl/core/IfNode.h"
#include "ndbl/core/FunctionNode.h"
#include "ndbl/core/LiteralNode.h"
#include "ndbl/core/Scope.h"
#include "ndbl/core/VariableNode.h"
#include "ndbl/core/WhileLoopNode.h"

#include "Instruction.h"
#include "Register.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

Code::Code(const Graph* graph )
: m_meta_data({graph})
{}

Instruction* Code::push_instr(OpCode _type)
{
    auto instr = new Instruction(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

bool Compiler::is_syntax_tree_valid(const Graph* _graph)
{
    if( _graph->is_empty())
        return false;

    const Nodlang* language = get_language();
    for( Node* node : _graph->nodes() )
    {
        switch ( node->type() )
        {
            case NodeType_VARIABLE:
            {
                if(node->scope() == nullptr )
                {
                    LOG_ERROR("Compiler", "\"%s\" should have a scope.\n", node->name().c_str() );
                    return false;
                }
                break;
            }

            case NodeType_OPERATOR:
            {
                auto* invokable = static_cast<const FunctionNode*>(node);
                if ( !language->find_operator_fct(invokable->get_func_type()) )
                {
                    std::string signature;
                    language->serialize_func_sig(signature, invokable->get_func_type());
                    LOG_ERROR("Compiler", "Operator is not declared: %s\n", signature.c_str());
                    return false;
                }
            }
            case NodeType_FUNCTION:
            {
                auto* invokable = static_cast<const FunctionNode*>(node);
                if ( !language->find_function(invokable->get_func_type()) )
                {
                    std::string signature;
                    language->serialize_func_sig(signature, invokable->get_func_type());
                    LOG_ERROR("Compiler", "Function is not declared: %s\n", signature.c_str());
                    return false;
                }
            }
        }
    }

    return true;
}

void Compiler::compile_input_slot( const Slot* slot)
{
    if( slot->empty() )
    {
        return;
    }
    ASSERT( slot->adjacent_count() == 1 );
    compile_output_slot( slot->first_adjacent() );
}

void Compiler::compile_output_slot(const Slot* slot)
{
    ASSERT(slot->has_flags(SlotFlag_OUTPUT) );
    compile_node(slot->node);
}

void Compiler::compile_inner_scope(const Node* node, bool _insert_fake_return)
{
    ASSERT( node );
    ASSERT(node->has_internal_scope() );

    // call push_stack_frame
    {
        Instruction *instr  = m_temp_code->push_instr(OpCode_push_stack_frame);
        instr->push.scope = node->internal_scope();
        char str[64];
        snprintf(str, 64, "%s's internal_scope", node->name().c_str());
        instr->m_comment = str;
    }

    // push each variable
    for(auto each_variable : node->internal_scope()->child_vars())
    {
        Instruction* instr   = m_temp_code->push_instr(OpCode_push_var);
        instr->push.var      = each_variable;
        instr->m_comment     = each_variable->name();
    }

    // compile content
    for( Node* each_node : node->internal_scope()->child() )
    {
        compile_node( each_node );
    }

    // before to pop, we could insert a return value
    if( _insert_fake_return )
    {
        m_temp_code->push_instr(OpCode_ret); // fake a return statement
    }

    // pop each variable
    for(auto each_variable : node->internal_scope()->child_vars())
    {
        Instruction *instr   = m_temp_code->push_instr(OpCode_pop_var);
        instr->push.var      = each_variable;
        instr->m_comment     = each_variable->name();
    }

    {
        Instruction *instr = m_temp_code->push_instr(OpCode_pop_stack_frame);
        instr->pop.scope   = node->internal_scope();
        instr->m_comment   = node->name() + "'s internal_scope";
    }
}

void Compiler::compile_node( const Node* _node )
{
    ASSERT( _node );

    switch (_node->type())
    {
        case NodeType_BLOCK_FOR_LOOP:
            compile_for_loop(static_cast<const ForLoopNode*>(_node));
            break;
        case NodeType_BLOCK_WHILE_LOOP:
            compile_while_loop(static_cast<const WhileLoopNode*>(_node));
            break;
        case NodeType_BLOCK_IF:
            compile_conditional_struct(static_cast<const IfNode*>(_node));
            break;
        default:
        {
            // Compile all the outputs connected to each _node inputs.
            for ( const Slot* slot: _node->filter_slots( SlotFlag_INPUT ) )
            {
                if( slot->adjacent_count() == 0)
                {
                    continue;
                }
                // Compile adjacent_output ( except if node is a Variable which is compiled once, see compile_variable_node() )
                Slot* adjacent_output = slot->first_adjacent();
                if ( !adjacent_output->node->get_class()->is<VariableNode>() )
                {
                    // Any other slot must be compiled recursively
                    compile_output_slot( adjacent_output );
                }
            }

            // eval node

            switch (_node->type())
            {
                case NodeType_FUNCTION:
                case NodeType_OPERATOR:
                {
                    Instruction*      instr     = m_temp_code->push_instr(OpCode_call);
                    const FunctionDescriptor*   func_type = static_cast<const FunctionNode*>(_node)->get_func_type();

                    const IInvokable* invokable = get_language()->find_function( func_type ); // Get exact OR fallback function (in case of arg cast)
                    VERIFY(invokable != nullptr, "Unable to find this function");
                    instr->call.invokable = invokable;

                    break;
                }
                case NodeType_LITERAL:
                case NodeType_VARIABLE:
                    VERIFY(false, "not implemented yet");
            }

        }
    }
}

void Compiler::compile_for_loop(const ForLoopNode* for_loop)
{
    // Compile initialization instruction
    compile_input_slot( for_loop->initialization_slot() );

    // compile condition and memorise its position
    u64_t conditionInstrLine = m_temp_code->get_next_index();
    compile_instruction_as_condition( for_loop->condition() );

    // jump if condition is not true
    Instruction* skipTrueBranch = m_temp_code->push_instr(OpCode_jne );
    skipTrueBranch->m_comment = "jump true branch";

    if ( auto true_branch = for_loop->branch_out(Branch_TRUE) )
    {
        compile_inner_scope( true_branch->node );

        // Compile iteration instruction
        compile_input_slot( for_loop->iteration_slot() );

        // jump back to condition instruction
        auto loopJump = m_temp_code->push_instr(OpCode_jmp );
        loopJump->jmp.offset = signed_diff( conditionInstrLine, loopJump->line );
        loopJump->m_comment = "jump back to \"for\"";
    }


    skipTrueBranch->jmp.offset = m_temp_code->get_next_index() - skipTrueBranch->line;
}

void Compiler::compile_while_loop(const WhileLoopNode* while_loop)
{
    // compile condition and memorise its position
    u64_t conditionInstrLine = m_temp_code->get_next_index();
    compile_instruction_as_condition( while_loop->condition() );

    // jump if condition is not true
    Instruction* skipTrueBranch = m_temp_code->push_instr(OpCode_jne );
    skipTrueBranch->m_comment = "jump if not equal";

    if ( auto whileScope = while_loop->branch_out( Branch_TRUE ) )
    {
        compile_inner_scope( whileScope->node );

        // jump back to condition instruction
        auto loopJump = m_temp_code->push_instr(OpCode_jmp );
        loopJump->jmp.offset = signed_diff( conditionInstrLine, loopJump->line );
        loopJump->m_comment = "jump back to \"while\"";
    }

    skipTrueBranch->jmp.offset = m_temp_code->get_next_index() - skipTrueBranch->line;
}

void Compiler::compile_instruction_as_condition(const Node* _instr_node)
{
    // compile condition result (must be stored in rax after this line)
    compile_node(_instr_node);

    // move "true" result to rdx
    Instruction* store_true = m_temp_code->push_instr(OpCode_mov);
    store_true->mov.src.b   = true;
    store_true->mov.dst.u8  = Register_rdx;
    store_true->m_comment   = "store true in rdx";

    // compare rax (condition result) with rdx (true)
    Instruction* cmp_instr  = m_temp_code->push_instr(OpCode_cmp);  // works only with registry
    cmp_instr->cmp.left.u8  = Register_rax;
    cmp_instr->cmp.right.u8 = Register_rdx;
    cmp_instr->m_comment    = "compare condition with rdx";
}

void Compiler::compile_conditional_struct(const IfNode* _cond_node)
{
    compile_instruction_as_condition( _cond_node->condition() ); // compile condition instruction, store result, compare

    Instruction* jump_over_true_branch = m_temp_code->push_instr(OpCode_jne);
    jump_over_true_branch->m_comment   = "conditional jump";

    Instruction* jump_after_conditional = nullptr;

    if ( auto true_branch = _cond_node->branch_out( Branch_TRUE ) )
    {
        compile_inner_scope( true_branch->node );

        if ( _cond_node->branch_out( Branch_FALSE )->node )
        {
            jump_after_conditional = m_temp_code->push_instr(OpCode_jmp);
            jump_after_conditional->m_comment = "jump after else";
        }
    }

    i64_t next_index = m_temp_code->get_next_index();
    jump_over_true_branch->jmp.offset = next_index - jump_over_true_branch->line;

    if ( const Slot* false_branch = _cond_node->branch_out(Branch_FALSE ) )
    {
        if( false_branch->node->get_class()->is<IfNode>() )
        {
            compile_conditional_struct( static_cast<const IfNode*>(false_branch->node) );
        }
        else
        {
            compile_inner_scope(false_branch->node );
        }

        if ( jump_after_conditional )
        {
            jump_after_conditional->jmp.offset = signed_diff(m_temp_code->get_next_index(), jump_after_conditional->line);
        }
    }
}

const Code* Compiler::compile_syntax_tree(const Graph* _graph)
{
    if (is_syntax_tree_valid(_graph))
    {
        m_temp_code = new Code( _graph );

        try
        {
            compile_inner_scope( _graph->root().get(), true); // "true" <== here is a hack, TODO: implement a real ReturnNode
            LOG_MESSAGE("Compiler", "Program compiled.\n");
        }
        catch ( const std::exception& e )
        {
            delete m_temp_code;
            m_temp_code = nullptr;
            LOG_ERROR("Compiler", "Unable to create_new assembly code for program. Reason: %s\n", e.what());
        }
        return m_temp_code;
    }
    return nullptr;
}
