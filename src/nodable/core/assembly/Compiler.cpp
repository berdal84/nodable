#include "Compiler.h"

#include <exception>
#include <iostream>

#include "fw/core/assertions.h"
#include "fw/core/log.h"
#include "fw/core/math.h"
#include "fw/core/string.h"
#include "fw/core/memory/Pool.h"

#include "nodable/core/ForLoopNode.h"
#include "nodable/core/Graph.h"
#include "nodable/core/IConditional.h"
#include "nodable/core/IfNode.h"
#include "nodable/core/InvokableComponent.h"
#include "nodable/core/LiteralNode.h"
#include "nodable/core/Scope.h"
#include "nodable/core/VariableNode.h"
#include "nodable/core/WhileLoopNode.h"

#include "Instruction.h"
#include "Register.h"

using namespace ndbl;
using namespace ndbl::assembly;
using namespace fw;

Code::Code( PoolID<Node> root )
: m_meta_data({root})
{}

Instruction* Code::push_instr(Instruction_t _type)
{
    auto instr = new Instruction(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

bool assembly::Compiler::is_syntax_tree_valid(const Graph* _graph)
{
    if( _graph->is_empty()) return false;

    const std::vector<PoolID<Node>>& nodes = _graph->get_node_registry();
    for( auto each_node : nodes )
    {
        // Check for undeclared variables
        if( auto scope = each_node->get_component<Scope>() )
        {
            const std::vector<PoolID<VariableNode> >& variables = scope->variables();

            for( auto each_variable : variables )
            {
                if( !each_variable->is_declared() )
                {
                    LOG_ERROR("Compiler", "Syntax error: \"%s\" is not declared.\n", each_variable->name.c_str() );
                    return false;
                }
            }
        }

        // Check for undeclared functions
        if( auto component = each_node->get_component<const InvokableComponent>() )
        {
            if ( !component->has_function() )
            {
                LOG_ERROR("Compiler", "Syntax error: \"%s\" is not a function available.\n", each_node->name.c_str() );
                return false;
            }
        }
    }

    return true;
}

void Compiler::compile_input_slot( const Slot& slot)
{
    if( slot.empty() )
    {
        return;
    }
    FW_ASSERT( slot.adjacent_count() == 1 )
    compile_output_slot( *slot.first_adjacent() );
}

void Compiler::compile_output_slot( const Slot& slot)
{
    FW_ASSERT(slot.has_flags(SlotFlag_OUTPUT) )
    Property* property = slot.get_property();
    FW_EXPECT(property != nullptr, "Vertex should contain a valid property id" )
    compile_node( slot.get_node() );
}

void assembly::Compiler::compile_scope(const Scope* _scope, bool _insert_fake_return)
{
    FW_ASSERT(_scope)
    const Node* scope_owner = _scope->get_owner().get();
    FW_ASSERT(scope_owner)

    // call push_stack_frame
    {
        Instruction *instr  = m_temp_code->push_instr(Instruction_t::push_stack_frame);
        instr->push.scope = _scope->poolid();
        char str[64];
        snprintf(str, 64, "%s's scope", scope_owner->name.c_str());
        instr->m_comment = str;
    }

    // push each variable
    for(auto each_variable : _scope->variables())
    {
        Instruction* instr   = m_temp_code->push_instr(Instruction_t::push_var);
        instr->push.var      = each_variable;
        instr->m_comment     = each_variable->name;
    }

    // compile content
    for( PoolID<Node> each_node : scope_owner->children() )
    {
        compile_node( each_node.get() );
    }

    // before to pop, we could insert a return value
    if( _insert_fake_return )
    {
        m_temp_code->push_instr(Instruction_t::ret); // fake a return statement
    }

    // pop each variable
    for(auto each_variable : _scope->variables())
    {
        Instruction *instr   = m_temp_code->push_instr(Instruction_t::pop_var);
        instr->push.var      = each_variable;
        instr->m_comment     = each_variable->name;
    }

    {
        Instruction *instr = m_temp_code->push_instr(Instruction_t::pop_stack_frame);
        instr->pop.scope   = _scope->poolid();
        instr->m_comment   = scope_owner->name + "'s scope";
    }
}

void assembly::Compiler::compile_node( const Node* _node )
{
    FW_ASSERT( _node )

    if ( _node->get_type()->is_child_of<IConditional>())
    {
        if ( auto for_loop = cast<const ForLoopNode>(_node))
        {
            compile_for_loop(for_loop);
        }
        else if ( auto while_loop = cast<const WhileLoopNode>(_node))
        {
            compile_while_loop(while_loop);
        }
        else if ( auto cond_struct_node = cast<const IfNode>(_node) )
        {
            compile_conditional_struct(cond_struct_node);
        }
        else
        {
            std::string message = "The class ";
            message.append(_node->get_type()->get_name());
            message.append(" is not handled by the compiler.");
            throw std::runtime_error(message);
        }
    }
    else
    {
        // Compile all the outputs connected to each _node inputs.
        for ( const Slot* slot: _node->filter_slots( SlotFlag_INPUT ) )
        {
            if( slot->adjacent_count() == 0)
            {
                continue;
            }
            // Compile adjacent_output ( except if node is a Variable which is compiled once, see compile_variable_node() )
            Slot* adjacent_output = slot->first_adjacent().get();
            if ( !adjacent_output->node->get_type()->is<VariableNode>() )
            {
                // Any other slot must be compiled recursively
                compile_output_slot( *adjacent_output );
            }
        }

        // eval node
        bool should_be_evaluated = _node->has_component<InvokableComponent>() || extends<VariableNode>(_node) ||
                                   extends<LiteralNode>(_node) ;
        if ( should_be_evaluated )
        {
            Instruction *instr = m_temp_code->push_instr(Instruction_t::eval_node);
            instr->eval.node   = _node->poolid();
            instr->m_comment   = _node->name;
        }

        // For instruction only: Copy node value to a register
        if( _node->is_instruction() )
        {
            const variant* root_node_value = _node->get_prop( VALUE_PROPERTY )->value();
            Instruction* instr     = m_temp_code->push_instr( Instruction_t::deref_qword );
            instr->uref.ptr        = root_node_value->data();
            instr->uref.type       = root_node_value->get_type();
            instr->m_comment       = "copy root's value ";
            instr->m_comment.append("(");
            instr->m_comment.append(instr->uref.type->get_name());
            instr->m_comment.append(")");
        }
    }
}

void assembly::Compiler::compile_for_loop(const ForLoopNode* for_loop)
{
    // Compile initialization instruction
    compile_input_slot( for_loop->initialization_slot() );

    // compile condition and memorise its position
    u64_t conditionInstrLine = m_temp_code->get_next_index();
    compile_instruction_as_condition( for_loop->get_condition().get() );

    // jump if condition is not true
    Instruction* skipTrueBranch = m_temp_code->push_instr( Instruction_t::jne );
    skipTrueBranch->m_comment = "jump true branch";

    if ( auto true_branch = for_loop->get_scope_at( Branch_TRUE ) )
    {
        compile_scope( true_branch.get() );

        // Compile iteration instruction
        compile_input_slot( for_loop->iteration_slot() );

        // jump back to condition instruction
        auto loopJump = m_temp_code->push_instr( Instruction_t::jmp );
        loopJump->jmp.offset = signed_diff( conditionInstrLine, loopJump->line );
        loopJump->m_comment = "jump back to \"for\"";
    }


    skipTrueBranch->jmp.offset = m_temp_code->get_next_index() - skipTrueBranch->line;
}

void assembly::Compiler::compile_while_loop(const WhileLoopNode* while_loop)
{
    // compile condition and memorise its position
    u64_t conditionInstrLine = m_temp_code->get_next_index();
    compile_instruction_as_condition( while_loop->get_condition().get() );

    // jump if condition is not true
    Instruction* skipTrueBranch = m_temp_code->push_instr( Instruction_t::jne );
    skipTrueBranch->m_comment = "jump if not equal";

    if ( auto whileScope = while_loop->get_scope_at( Branch_TRUE ) )
    {
        compile_scope( whileScope.get() );

        // jump back to condition instruction
        auto loopJump = m_temp_code->push_instr( Instruction_t::jmp );
        loopJump->jmp.offset = signed_diff( conditionInstrLine, loopJump->line );
        loopJump->m_comment = "jump back to \"while\"";
    }

    skipTrueBranch->jmp.offset = m_temp_code->get_next_index() - skipTrueBranch->line;
}

void assembly::Compiler::compile_instruction_as_condition( const Node* _instr_node)
{
    // compile condition result (must be stored in rax after this line)
    compile_node(_instr_node);

    // move "true" result to rdx
    Instruction* store_true = m_temp_code->push_instr(Instruction_t::mov);
    store_true->mov.src.set(true);
    store_true->mov.dst.set((u8_t)Register::rdx);
    store_true->m_comment = "store true in rdx";

    // compare rax (condition result) with rdx (true)
    Instruction* cmp_instr  = m_temp_code->push_instr(Instruction_t::cmp);  // works only with registry
    cmp_instr->cmp.left.set( (u8_t)Register::rax);
    cmp_instr->cmp.right.set( (u8_t)Register::rdx);
    cmp_instr->m_comment = "compare condition with rdx";
}

void assembly::Compiler::compile_conditional_struct(const IfNode* _cond_node)
{
    compile_instruction_as_condition(_cond_node->get_condition().get()); // compile condition instruction, store result, compare

    Instruction* jump_over_true_branch = m_temp_code->push_instr(Instruction_t::jne);
    jump_over_true_branch->m_comment   = "conditional jump";

    Instruction* jump_after_conditional = nullptr;

    if ( auto true_branch = _cond_node->get_scope_at( Branch_TRUE ) )
    {
        compile_scope( true_branch.get() );

        if ( _cond_node->get_scope_at( Branch_FALSE ) )
        {
            jump_after_conditional = m_temp_code->push_instr(Instruction_t::jmp);
            jump_after_conditional->m_comment = "jump after else";
        }
    }

    i64_t next_index = m_temp_code->get_next_index();
    jump_over_true_branch->jmp.offset = next_index - jump_over_true_branch->line;

    if ( Scope* false_scope = _cond_node->get_scope_at( Branch_FALSE ).get() )
    {
        if( false_scope->get_owner()->get_type()->is<IfNode>() )
        {
            auto* conditional_struct = cast<const IfNode>(false_scope->get_owner().get());
            compile_conditional_struct(conditional_struct);
        }
        else
        {
            compile_scope(false_scope);
        }

        if ( jump_after_conditional )
        {
            jump_after_conditional->jmp.offset = i64_t(m_temp_code->get_next_index()) - jump_after_conditional->line;
        }
    }
}

const Code* assembly::Compiler::compile_syntax_tree(const Graph* _graph)
{
    if (is_syntax_tree_valid(_graph))
    {
        m_temp_code = new Code( _graph->get_root() );

        try
        {
            Scope* scope = _graph->get_root()->get_component<Scope>().get();
            FW_ASSERT(scope)
            compile_scope(scope, true); // <--- true here is a hack, TODO: implement a real ReturnNode
            LOG_MESSAGE("Compiler", "Program compiled.\n");
        }
        catch ( const std::exception& e )
        {
            delete m_temp_code;
            m_temp_code = nullptr;
            LOG_ERROR("Compiler", "Unable to create assembly code for program. Reason: %s\n", e.what());
        }
        return m_temp_code;
    }
    return nullptr;
}
