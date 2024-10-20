#include "ASTCompiler.h"

#include <exception>
#include <iostream>

#include "tools/core/assertions.h"
#include "tools/core/log.h"
#include "tools/core/math.h"
#include "tools/core/string.h"
#include "tools/core/memory/memory.h"

#include "ndbl/core/ASTForLoopNode.h"
#include "ndbl/core/ASTGraph.h"
#include "ndbl/core/ASTConditionalNode.h"
#include "ndbl/core/ASTFunctionNode.h"
#include "ndbl/core/ASTLiteralNode.h"
#include "ndbl/core/ASTScope.h"
#include "ndbl/core/ASTVariableNode.h"
#include "ndbl/core/ASTWhileLoopNode.h"

#include "Instruction.h"
#include "Register.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

Code::Code(const ASTGraph* graph )
: m_meta_data({graph})
{}

Instruction* Code::push_instr(OpCode _type)
{
    auto instr = new Instruction(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

bool ASTCompiler::is_syntax_tree_valid(const ASTGraph* _graph)
{
    if( _graph->is_empty())
        return false;

    const Nodlang* language = get_language();
    const std::vector<ASTNode*>& nodes = _graph->get_node_registry();
    for( auto each_node : nodes )
    {
        switch ( each_node->type() )
        {
            case ASTNodeType_BLOCK_SCOPE:
            {
                auto scope = each_node->get_component<ASTScope>();
                const std::vector<ASTVariableNode*>& variables = scope->variables();

                for( auto each_variable : variables )
                {
                    if( each_variable->get_scope() == nullptr )
                    {
                        LOG_ERROR("Compiler", "\"%s\" should have a scope.\n", each_variable->name().c_str() );
                        return false;
                    }
                }
                break;
            }

            case ASTNodeType_OPERATOR:
            {
                auto* invokable = static_cast<const ASTFunctionNode*>(each_node);
                if ( !language->find_operator_fct(invokable->get_func_type()) )
                {
                    std::string signature;
                    language->serialize_func_sig(signature, invokable->get_func_type());
                    LOG_ERROR("Compiler", "Operator is not declared: %s\n", signature.c_str());
                    return false;
                }
            }
            case ASTNodeType_FUNCTION:
            {
                auto* invokable = static_cast<const ASTFunctionNode*>(each_node);
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

void ASTCompiler::compile_input_slot(const Slot& slot)
{
    if( slot.empty() )
    {
        return;
    }
    ASSERT( slot.adjacent_count() == 1 )
    compile_output_slot( *slot.first_adjacent() );
}

void ASTCompiler::compile_output_slot(const Slot& slot)
{
    ASSERT(slot.has_flags(SlotFlag_OUTPUT) )
    compile_node(slot.node() );
}

void ASTCompiler::compile_scope(const ASTScope* _scope, bool _insert_fake_return)
{
    ASSERT(_scope)
    const ASTNode* scope_owner = _scope->get_owner();
    ASSERT(scope_owner)

    // call push_stack_frame
    {
        Instruction *instr  = m_temp_code->push_instr(OpCode_push_stack_frame);
        instr->push.scope = _scope;
        char str[64];
        snprintf(str, 64, "%s's scope", scope_owner->name().c_str());
        instr->m_comment = str;
    }

    // push each variable
    for(auto each_variable : _scope->variables())
    {
        Instruction* instr   = m_temp_code->push_instr(OpCode_push_var);
        instr->push.var      = each_variable;
        instr->m_comment     = each_variable->name();
    }

    // compile content
    for( ASTNode* each_node : scope_owner->children() )
    {
        compile_node( each_node );
    }

    // before to pop, we could insert a return value
    if( _insert_fake_return )
    {
        m_temp_code->push_instr(OpCode_ret); // fake a return statement
    }

    // pop each variable
    for(auto each_variable : _scope->variables())
    {
        Instruction *instr   = m_temp_code->push_instr(OpCode_pop_var);
        instr->push.var      = each_variable;
        instr->m_comment     = each_variable->name();
    }

    {
        Instruction *instr = m_temp_code->push_instr(OpCode_pop_stack_frame);
        instr->pop.scope   = _scope;
        instr->m_comment   = scope_owner->name() + "'s scope";
    }
}

void ASTCompiler::compile_node(const ASTNode* _node )
{
    ASSERT( _node )

    switch (_node->type())
    {
        case ASTNodeType_BLOCK_FOR_LOOP:
            compile_for_loop(static_cast<const ASTForLoopNode*>(_node));
            break;
        case ASTNodeType_BLOCK_WHILE_LOOP:
            compile_while_loop(static_cast<const ASTWhileLoopNode*>(_node));
            break;
        case ASTNodeType_BLOCK_CONDITION:
            compile_conditional_struct(static_cast<const ASTConditionalNode*>(_node));
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
                if ( !adjacent_output->node()->get_class()->is<ASTVariableNode>() )
                {
                    // Any other slot must be compiled recursively
                    compile_output_slot( *adjacent_output );
                }
            }

            // eval node

            switch (_node->type())
            {
                case ASTNodeType_FUNCTION:
                case ASTNodeType_OPERATOR:
                {
                    Instruction*      instr     = m_temp_code->push_instr(OpCode_call);
                    const FunctionDescriptor*   func_type = static_cast<const ASTFunctionNode*>(_node)->get_func_type();

                    const IInvokable* invokable = get_language()->find_function( func_type ); // Get exact OR fallback function (in case of arg cast)
                    VERIFY(invokable != nullptr, "Unable to find this function")
                    instr->call.invokable = invokable;

                    break;
                }
                case ASTNodeType_LITERAL:
                case ASTNodeType_VARIABLE:
                    VERIFY(false, "not implemented yet")
            }

        }
    }
}

void ASTCompiler::compile_for_loop(const ASTForLoopNode* for_loop)
{
    // Compile initialization instruction
    compile_input_slot( for_loop->initialization_slot() );

    // compile condition and memorise its position
    u64_t conditionInstrLine = m_temp_code->get_next_index();
    compile_instruction_as_condition(for_loop->condition(Branch_TRUE));

    // jump if condition is not true
    Instruction* skipTrueBranch = m_temp_code->push_instr(OpCode_jne );
    skipTrueBranch->m_comment = "jump true branch";

    if ( auto true_branch = for_loop->scope_at( Branch_TRUE ) )
    {
        compile_scope( true_branch );

        // Compile iteration instruction
        compile_input_slot( for_loop->iteration_slot() );

        // jump back to condition instruction
        auto loopJump = m_temp_code->push_instr(OpCode_jmp );
        loopJump->jmp.offset = signed_diff( conditionInstrLine, loopJump->line );
        loopJump->m_comment = "jump back to \"for\"";
    }


    skipTrueBranch->jmp.offset = m_temp_code->get_next_index() - skipTrueBranch->line;
}

void ASTCompiler::compile_while_loop(const ASTWhileLoopNode* while_loop)
{
    // compile condition and memorise its position
    u64_t conditionInstrLine = m_temp_code->get_next_index();
    compile_instruction_as_condition(while_loop->condition(Branch_TRUE));

    // jump if condition is not true
    Instruction* skipTrueBranch = m_temp_code->push_instr(OpCode_jne );
    skipTrueBranch->m_comment = "jump if not equal";

    if ( auto whileScope = while_loop->scope_at( Branch_TRUE ) )
    {
        compile_scope( whileScope );

        // jump back to condition instruction
        auto loopJump = m_temp_code->push_instr(OpCode_jmp );
        loopJump->jmp.offset = signed_diff( conditionInstrLine, loopJump->line );
        loopJump->m_comment = "jump back to \"while\"";
    }

    skipTrueBranch->jmp.offset = m_temp_code->get_next_index() - skipTrueBranch->line;
}

void ASTCompiler::compile_instruction_as_condition(const ASTNode* _instr_node)
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

void ASTCompiler::compile_conditional_struct(const ASTConditionalNode* _cond_node)
{
    compile_instruction_as_condition(_cond_node->condition(Branch_TRUE)); // compile condition instruction, store result, compare

    Instruction* jump_over_true_branch = m_temp_code->push_instr(OpCode_jne);
    jump_over_true_branch->m_comment   = "conditional jump";

    Instruction* jump_after_conditional = nullptr;

    if ( auto true_branch = _cond_node->scope_at( Branch_TRUE ) )
    {
        compile_scope( true_branch );

        if ( _cond_node->scope_at( Branch_FALSE ) )
        {
            jump_after_conditional = m_temp_code->push_instr(OpCode_jmp);
            jump_after_conditional->m_comment = "jump after else";
        }
    }

    i64_t next_index = m_temp_code->get_next_index();
    jump_over_true_branch->jmp.offset = next_index - jump_over_true_branch->line;

    if ( ASTScope* false_scope = _cond_node->scope_at(Branch_FALSE ) )
    {
        if( false_scope->get_owner()->get_class()->is<ASTConditionalNode>() )
        {
            auto* conditional_struct = cast<const ASTConditionalNode>(false_scope->get_owner());
            compile_conditional_struct(conditional_struct);
        }
        else
        {
            compile_scope(false_scope);
        }

        if ( jump_after_conditional )
        {
            jump_after_conditional->jmp.offset = signed_diff(m_temp_code->get_next_index(), jump_after_conditional->line);
        }
    }
}

const Code* ASTCompiler::compile_ast(const ASTGraph* _graph)
{
    if (is_syntax_tree_valid(_graph))
    {
        m_temp_code = new Code( _graph );

        try
        {
            ASTScope* scope = _graph->get_root()->get_component<ASTScope>();
            ASSERT(scope)
            compile_scope(scope, true); // <--- true here is a hack, TODO: implement a real ReturnNode
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
