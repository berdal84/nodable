#include "nodable/core/assembly/Compiler.h"

#include <memory>
#include <exception>
#include <iostream>

#include "nodable/core/String.h"
#include "nodable/core/assertions.h"
#include "nodable/core/VariableNode.h"
#include "nodable/core/Log.h"
#include "nodable/core/IConditionalStruct.h"
#include "nodable/core/ForLoopNode.h"
#include "nodable/core/Scope.h"
#include "nodable/core/InstructionNode.h"
#include "nodable/core/LiteralNode.h"
#include "nodable/core/InvokableComponent.h"
#include "nodable/core/GraphNode.h"
#include "nodable/core/math.h"

using namespace Nodable;
using namespace Nodable::assembly;

Instruction* Code::push_instr(opcode_t _type)
{
    auto instr = new Instruction(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

bool assembly::Compiler::is_syntax_tree_valid(const GraphNode* _graph)
{
    if( _graph->is_empty())
    {
        return false;
    }

    const NodeVec& nodes = _graph->get_node_registry();
    for( auto each_node : nodes )
    {
        // Check for undeclared variables
        if( const Scope* scope = each_node->get<Scope>())
        {
            const VariableNodeVec& vars = scope->get_variables();

            for(const VariableNode* each_variable : vars)
            {
                if( !each_variable->is_declared() )
                {
                    LOG_ERROR("Compiler", "Syntax error: %s is not declared.\n", each_variable->get_name() );
                    return false;
                }
            }
        }

        // Check for undeclared functions
        if( const InvokableComponent* component = each_node->get<InvokableComponent>() )
        {
            if ( !component->has_function() )
            {
                LOG_ERROR("Compiler", "Syntax error: %s is not a function available.\n", each_node->get_label() );
                return false;
            }
        }
    }

    return true;
}

void assembly::Compiler::compile(const Member * _member )
{
    NODABLE_ASSERT(_member);
    {
        if ( _member->get_type() == type::get<Node*>() )
        {
            compile((const Node *) *_member);
        }
        else if ( Member* input = _member->get_input() )
        {
            /*
             * if the member has an input it means it is not a simple literal value and we have to compile it.
             * In order to do that, we traverse the syntax tree starting from the node connected to it.
             * Once we have the list of the nodes to be updated, we loop on them.
             */
            compile(input->get_owner());
        }
    }
}

void assembly::Compiler::compile(const Scope* _scope, bool _insert_fake_return)
{
    if( !_scope)
    {
        LOG_VERBOSE("Compiler", "Ignoring nullptr Scope.\n")
        return;
    }

    Node* scope_owner = _scope->get_owner();
    NODABLE_ASSERT(scope_owner)

    // call push_stack_frame
    {
        Instruction *instr  = m_temp_code->push_instr(opcode_t::push_stack_frame);
        instr->push.scope = _scope;
        char str[64];
        snprintf(str, 64, "%s's scope", scope_owner->get_short_label());
        instr->m_comment = str;
    }

    // push each variable
    for(const VariableNode* each_variable : _scope->get_variables())
    {
        Instruction *instr   = m_temp_code->push_instr(opcode_t::push_var);
        instr->push.var      = each_variable;
        instr->m_comment     = std::string{each_variable->get_label()};
    }

    // compile content
    for( const Node* each_node : scope_owner->children_slots().content() )
    {
        compile(each_node);
    }

    // pop each variable
    for(const VariableNode* each_variable : _scope->get_variables())
    {
        Instruction *instr   = m_temp_code->push_instr(opcode_t::pop_var);
        instr->push.var      = each_variable;
        instr->m_comment     = std::string{each_variable->get_label()};
    }

    // call pop_stack_frame
    if( _insert_fake_return )
    {
        m_temp_code->push_instr(opcode_t::ret); // fake a return statement
    }

    {
        Instruction *instr     = m_temp_code->push_instr(opcode_t::pop_stack_frame);
        instr->pop.scope = _scope;
        instr->m_comment = std::string{scope_owner->get_short_label()} + "'s scope";
    }
}

void assembly::Compiler::compile(const Node* _node)
{
    if( !_node)
    {
        LOG_VERBOSE("Compiler", "Ignoring nullptr Node.\n")
        return;
    }

    if ( _node->get_type().is_child_of( type::get<IConditionalStruct>() ))
    {
        if ( const ForLoopNode* for_loop = _node->as<ForLoopNode>())
        {
            compile(for_loop);
        }
        else if ( const ConditionalStructNode* cond_struct_node = _node->as<ConditionalStructNode>())
        {
            compile(cond_struct_node);
        }
        else
        {
            std::string message = "The class ";
            message.append(_node->get_type().get_name());
            message.append(" is not handled by the compiler.");
            throw std::runtime_error(message);
        }
    }
    else if ( const InstructionNode* instr_node = _node->as<InstructionNode>() )
    {
        compile(instr_node);
    }
    else
    {
        // eval inputs
        for ( const Node* each_input : _node->inputs().content() )
        {
            if ( !each_input->is<VariableNode>() )
            {
                compile(each_input);
            }
        }

        // eval node
        bool should_be_evaluated = _node->has<InvokableComponent>() || _node->is<VariableNode>() || _node->is<LiteralNode>();
        if ( should_be_evaluated )
        {
            Instruction *instr = m_temp_code->push_instr(opcode_t::eval_node);
            instr->eval.node   = _node;

            if( auto variable = _node->as<VariableNode>())
            {
                instr->m_comment = variable->get_name();
            }
            else
            {
                instr->m_comment = _node->get_label();
            }
            // result is not stored, because this is necessary only for instruction's root node.
        }
    }

}

void assembly::Compiler::compile(const ForLoopNode* for_loop)
{
    // for_loop init instruction
    compile(for_loop->get_init_instr());

    u64_t condition_instr_line = m_temp_code->get_next_index();

    compile_as_condition(for_loop->get_cond_instr());

    Instruction* skip_true_branch = m_temp_code->push_instr(opcode_t::jne);
    skip_true_branch->m_comment = "jump if not equal";

    if ( auto true_scope = for_loop->get_condition_true_scope() )
    {
        compile(true_scope);

        // insert end-loop instruction.
        compile(for_loop->get_iter_instr());

        // insert jump to condition instructions.
        auto loop_jump = m_temp_code->push_instr(opcode_t::jmp);
        loop_jump->jmp.offset = math::signed_diff(condition_instr_line, loop_jump->line);
        loop_jump->m_comment  = "jump back to for";
    }

    skip_true_branch->jmp.offset = m_temp_code->get_next_index() - skip_true_branch->line;
}

void assembly::Compiler::compile_as_condition(const InstructionNode* _instr_node)
{
    // compile condition result (must be stored in rax after this line)
    compile(_instr_node);

    // move "true" result to rdx
    Instruction* store_true   = m_temp_code->push_instr(opcode_t::mov);
    store_true->mov.src.b     = true;
    store_true->mov.dst.r     = rdx;
    store_true->m_comment     = "store true";

    // compare rax (condition result) with rdx (true)
    Instruction* cmp_instr = m_temp_code->push_instr(opcode_t::cmp);  // works only with registry
    cmp_instr->cmp.left.r  = rax;
    cmp_instr->cmp.right.r = rdx;
    cmp_instr->m_comment   = "compare registers";
}

void assembly::Compiler::compile(const ConditionalStructNode* _cond_node)
{
    compile_as_condition(_cond_node->get_cond_instr()); // compile condition isntruction, store result, compare

    Instruction* jump_over_true_branch = m_temp_code->push_instr(opcode_t::jne);
    jump_over_true_branch->m_comment   = "jump if not equals";

    Instruction* jump_after_conditional = nullptr;

    if ( auto true_scope = _cond_node->get_condition_true_scope() )
    {
        compile(true_scope);

        if ( _cond_node->get_condition_false_scope() )
        {
            jump_after_conditional = m_temp_code->push_instr(opcode_t::jmp);
            jump_after_conditional->m_comment = "jump";
        }
    }

    jump_over_true_branch->jmp.offset = i64_t(m_temp_code->get_next_index()) - jump_over_true_branch->line;

    if ( auto false_scope = _cond_node->get_condition_false_scope() )
    {
        if( _cond_node->has_elseif() )
        {
            auto* else_if = false_scope->get_owner()->as<ConditionalStructNode>();
            compile(else_if);
        }
        else
        {
            compile(false_scope);
        }

        if ( jump_after_conditional )
        {
            jump_after_conditional->jmp.offset = i64_t(m_temp_code->get_next_index()) - jump_after_conditional->line;
        }
    }
}

void assembly::Compiler::compile(const InstructionNode *instr_node)
{
    const Member* root_node_member = instr_node->get_root_node_member();
    NODABLE_ASSERT(root_node_member)

    // copy instruction result to rax register
    if ( root_node_member->has_input_connected() )
    {
        compile(root_node_member);

        Node*   root_node       = root_node_member->get_input()->get_owner();
        Member* root_node_value = root_node->props()->get(k_value_member_name);

        if ( root_node_value )
        {
            Instruction* instr     = m_temp_code->push_instr(opcode_t::deref_ptr);
            instr->uref.qword_ptr  = &root_node_value->get_underlying_data();
            instr->uref.qword_type = &root_node_value->get_type();
            instr->m_comment       = "copy unreferenced data";
        }
    }
}

std::unique_ptr<const Code> assembly::Compiler::compile_syntax_tree(const GraphNode* _graph)
{
    if (is_syntax_tree_valid(_graph))
    {
        Node* root = _graph->get_root();

        m_temp_code = std::make_unique<Code>(root);

        try
        {
            auto scope = root->get<Scope>();
            NODABLE_ASSERT(scope)
            compile(scope, true); // <--- true here is a hack, TODO: implement a real ReturnNode
            LOG_MESSAGE("Compiler", "Program compiled.\n");
        }
        catch ( const std::exception& e )
        {
            m_temp_code.reset();
            LOG_ERROR("Compiler", "Unable to create assembly code for program. Reason: %s\n", e.what());
        }
        return std::move(m_temp_code);
    }
    return nullptr;
}
