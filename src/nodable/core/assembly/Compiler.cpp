#include "Compiler.h"
#include <exception>
#include <iostream>

#include "fw/core/log.h"
#include "fw/core/string.h"
#include "fw/core/assertions.h"
#include "fw/core/math.h"

#include "core/ConditionalStructNode.h"
#include "core/ForLoopNode.h"
#include "core/WhileLoopNode.h"
#include "core/Graph.h"
#include "core/IConditionalStruct.h"
#include "core/InstructionNode.h"
#include "core/InvokableComponent.h"
#include "core/LiteralNode.h"
#include "core/Scope.h"
#include "core/VariableNode.h"

#include "Register.h"
#include "Instruction.h"
#include "core/Pool.h"

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

void Compiler::compile_output_slot( const Slot* slot)
{
    FW_ASSERT(slot != nullptr)
    FW_ASSERT(slot->flags & SlotFlag_OUTPUT)
    Property* property = slot->get_property();
    FW_EXPECT(property != nullptr, "Vertex should contain a valid property id" )
    compile_node( slot->node );
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
        compile_node(each_node);
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

void assembly::Compiler::compile_node(PoolID<const Node> node_id)
{
    FW_ASSERT(node_id)

    const Node* _node = node_id.get();
    if ( _node->get_type()->is_child_of<IConditionalStruct>())
    {
        if ( auto for_loop = fw::cast<const ForLoopNode>(_node))
        {
            compile_for_loop(for_loop);
        }
        else if ( auto while_loop = fw::cast<const WhileLoopNode>(_node))
        {
            compile_while_loop(while_loop);
        }
        else if ( auto cond_struct_node = fw::cast<const ConditionalStructNode>(_node) )
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
    else if ( auto instr_node = fw::cast<const InstructionNode>(_node) )
    {
        compile_instruction(instr_node);
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
                compile_output_slot( adjacent_output );
            }
        }

        // eval node
        bool should_be_evaluated = _node->has_component<InvokableComponent>() || fw::extends<VariableNode>(_node) ||
                                   fw::extends<LiteralNode>(_node) ;
        if ( should_be_evaluated )
        {
            Instruction *instr = m_temp_code->push_instr(Instruction_t::eval_node);
            instr->eval.node   = _node->poolid();
            instr->m_comment   = _node->name;
        }
    }

}

void assembly::Compiler::compile_for_loop(const ForLoopNode* for_loop)
{
    // for_loop init instruction
    compile_instruction(for_loop->init_instr.get());

    // compile condition and memorise its position
    u64_t condition_instr_line = m_temp_code->get_next_index();
    compile_instruction_as_condition(for_loop->cond_instr.get());

    // jump if condition is not true
    Instruction* skip_true_branch = m_temp_code->push_instr(Instruction_t::jne);
    skip_true_branch->m_comment = "jump if not equal";

    if ( auto true_scope = for_loop->get_condition_true_scope().get() )
    {
        compile_scope(true_scope);

        // insert end-loop instruction.
        compile_instruction(for_loop->iter_instr.get());

        // insert jump to condition instructions.
        auto loop_jump = m_temp_code->push_instr(Instruction_t::jmp);
        loop_jump->jmp.offset = math::signed_diff(condition_instr_line, loop_jump->line);
        loop_jump->m_comment  = "jump back to for";
    }

    skip_true_branch->jmp.offset = m_temp_code->get_next_index() - skip_true_branch->line;
}

void assembly::Compiler::compile_while_loop(const WhileLoopNode*while_loop)
{
    // compile condition and memorise its position
    u64_t condition_instr_line = m_temp_code->get_next_index();
    compile_instruction_as_condition(while_loop->cond_instr.get());

    // jump if condition is not true
    Instruction* skip_true_branch = m_temp_code->push_instr(Instruction_t::jne);
    skip_true_branch->m_comment = "jump if not equal";

    if ( Scope* true_scope = while_loop->get_condition_true_scope().get() )
    {
        compile_scope(true_scope);

        // jump back to condition instruction
        auto loop_jump = m_temp_code->push_instr(Instruction_t::jmp);
        loop_jump->jmp.offset = math::signed_diff(condition_instr_line, loop_jump->line);
        loop_jump->m_comment  = "jump back to for";
    }

    skip_true_branch->jmp.offset = m_temp_code->get_next_index() - skip_true_branch->line;
}

void assembly::Compiler::compile_instruction_as_condition(const InstructionNode* _instr_node)
{
    // compile condition result (must be stored in rax after this line)
    compile_instruction(_instr_node);

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

void assembly::Compiler::compile_conditional_struct(const ConditionalStructNode* _cond_node)
{
    compile_instruction_as_condition(_cond_node->cond_expr.get()); // compile condition instruction, store result, compare

    Instruction* jump_over_true_branch = m_temp_code->push_instr(Instruction_t::jne);
    jump_over_true_branch->m_comment   = "conditional jump";

    Instruction* jump_after_conditional = nullptr;

    if ( Scope* true_scope = _cond_node->get_condition_true_scope().get() )
    {
        compile_scope(true_scope);

        if ( _cond_node->get_condition_false_scope() )
        {
            jump_after_conditional = m_temp_code->push_instr(Instruction_t::jmp);
            jump_after_conditional->m_comment = "jump after else";
        }
    }

    i64_t next_index = m_temp_code->get_next_index();
    jump_over_true_branch->jmp.offset = next_index - jump_over_true_branch->line;

    if ( Scope* false_scope = _cond_node->get_condition_false_scope().get() )
    {
        if( _cond_node->is_chain() )
        {
            auto* conditional_struct = fw::cast<const ConditionalStructNode>(false_scope->get_owner().get());
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

void assembly::Compiler::compile_instruction(const InstructionNode *instr_node)
{
    const Slot& instruction_input = instr_node->root_slot();

    // Early return if nothing is connected
    Slot* expression_output = instruction_input.first_adjacent().get();
    if( expression_output == nullptr )
    {
        return;
    }

    // Compiles input
    compile_output_slot( expression_output );

    // Add instruction to copy the result in a register
    variant* root_node_value = expression_output->get_node()->get_prop( VALUE_PROPERTY )->value();
    Instruction* instr     = m_temp_code->push_instr( Instruction_t::deref_qword );
    instr->uref.ptr        = root_node_value->data();
    instr->uref.type       = root_node_value->get_type();
    instr->m_comment       = "copy root's value ";
    instr->m_comment.append("(");
    instr->m_comment.append(instr->uref.type->get_name());
    instr->m_comment.append(")");
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
