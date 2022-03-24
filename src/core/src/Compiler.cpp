#include <nodable/core/Compiler.h>

#include <memory>
#include <iostream>

#include <nodable/core/String.h>
#include <nodable/core/assertions.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/IConditionalStruct.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/Scope.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/LiteralNode.h>
#include <nodable/core/InvokableComponent.h>

using namespace Nodable;
using namespace Nodable::Asm;

i64 signed_diff(u64 _left, u64 _right)
{
    bool left_greater_than_right = _left > _right;
    u64 abs_diff = left_greater_than_right ? (_left - _right) : (_right - _left);
    NODABLE_ASSERT( abs_diff <= std::numeric_limits<u64>::max() );
    return left_greater_than_right ? (i64)abs_diff : -(i64)abs_diff;
}

std::string Asm::Instr::to_string(const Instr& _instr)
{
    std::string result;
    result.reserve(60);

    // append "<line> :"
    std::string str = std::to_string(_instr.m_line);
    while( str.length() < 4 )
        str.append(" ");
    result.append( str );
    result.append( ": " );

    // append instruction type
    result.append( Asm::to_string(_instr.m_type));
    result.append( " " );

    // optionally append parameters
    switch ( _instr.m_type )
    {
        case Instr_t::call:
        {
            FctId fct_id   = (FctId)_instr.m_arg0;
            Member* member = (Member*)_instr.m_arg1;
            result.append( Asm::to_string(fct_id) );
            while( result.length() < 30 )
                result.append(" ");
            result.append( String::address_to_hexadecimal(member) );
            break;
        }

        case Instr_t::mov:
        case Instr_t::cmp:
        {
            result.append("%");
            result.append(Asm::to_string( (Register)_instr.m_arg0 ));
            result.append(", %");
            result.append(Asm::to_string( (Register)_instr.m_arg1 ));
            break;
        }

        case Instr_t::jne:
        case Instr_t::jmp:
        {
            result.append( std::to_string( _instr.m_arg0 ) );
            break;
        }

        case Instr_t::ret: // nothing else to do.
            break;
    }

    // optionally append comment
    if ( !_instr.m_comment.empty() )
    {
        while( result.length() < 50 ) // align on 80th char
            result.append(" ");
        result.append( "; " );
        result.append( _instr.m_comment );
    }
    return result;
}

Code::~Code()
{
    for( auto each : m_instructions )
        delete each;
    m_instructions.clear();
    m_instructions.resize(0);
}

Instr* Code::push_instr(Instr_t _type)
{
    Instr* instr = new Instr(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

bool Asm::Compiler::is_program_valid(const Node* _program_graph_root)
{
    bool is_valid;

    // check if program an be run
    const VariableNodes& vars = _program_graph_root->get<Scope>()->get_variables();
    bool found_a_var_uninit = false;
    auto it = vars.begin();
    while(!found_a_var_uninit && it != vars.end() )
    {
        if( !(*it)->is_declared() )
        {
            LOG_ERROR("Compiler", "Unable to load program because %s is not declared.\n", (*it)->get_name() );
            found_a_var_uninit = true;
        }
        ++it;
    }

    is_valid = !found_a_var_uninit;

    return is_valid;
}

void Asm::Compiler::compile_member(const Member * _member )
{
    NODABLE_ASSERT(_member);
    {
        if (_member->is_meta_type( R::get_meta_type<Node *>() ) )
        {
            compile_node((const Node *) *_member);
        }
        else if ( Member* input = _member->get_input() )
        {
            /*
             * if the member has an input it means it is not a simple literal value and we have to compile it.
             * In order to do that, we traverse the syntax tree starting from the node connected to it.
             * Once we have the list of the nodes to be updated, we loop on them.
             */
            compile_node(input->get_owner());
        }

        {
            Instr *instr  = m_temp_code->push_instr(Instr_t::call);
            instr->m_arg0 = (u64) FctId::store_data_ptr;
            instr->m_arg1 = (u64) _member;
            char str[128];
            sprintf(str
                    , "%s -> %s"
                    , _member->get_owner()->get_label()
                    , _member->get_name().c_str());
            instr->m_comment = str;
        }
    }
}

void Asm::Compiler::compile_scope(const Scope* _scope, bool _insert_fake_return)
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
        Instr *instr  = m_temp_code->push_instr(Instr_t::call);
        instr->m_arg0 = (u64) FctId::push_frame;
        instr->m_arg1 = (u64) _scope;
        char str[64];
        snprintf(str, 64, "%s's scope", scope_owner->get_short_label());
        instr->m_comment = str;
    }

    // push each varaible onto the stack
    for(const VariableNode* each_variable : _scope->get_variables())
    {
        Instr *instr     = m_temp_code->push_instr(Instr_t::call);
        instr->m_arg0    = (u64)FctId::push_variable;
        instr->m_arg1    = (u64)each_variable;
        instr->m_comment = std::string{each_variable->get_label()};
    }

    // compile content
    for( const Node* each_node : scope_owner->children_slots().content() )
    {
        compile_node(each_node);
    }

    // call pop_stack_frame
    if( _insert_fake_return )
    {
        m_temp_code->push_instr(Instr_t::ret); // fake a return statement
    }

    {
        Instr *instr     = m_temp_code->push_instr(Instr_t::call);
        instr->m_arg0    = (u64) FctId::pop_frame;
        instr->m_arg1    = (u64) scope_owner;
        instr->m_comment = std::string{scope_owner->get_short_label()} + "'s scope";
    }
}

void Asm::Compiler::compile_node(const Node* _node)
{
    if( !_node)
    {
        LOG_VERBOSE("Compiler", "Ignoring nullptr Node.\n")
        return;
    }

    if ( _node->is<IConditionalStruct>() )
    {
        const IConditionalStruct* i_cond_struct = _node->as<IConditionalStruct>();

        // for_loop init instruction
        if ( auto for_loop = _node->as<ForLoopNode>() )
        {
            compile_member(for_loop->get_init_expr());
        }

        u64 condition_instr_line = m_temp_code->get_next_index();

        const Member* condition_member = i_cond_struct->condition_member();

        if (condition_member->is_initialized() )
        {
            NODABLE_ASSERT(condition_member)
            compile_member(condition_member);
        }

        Instr* store_instr     = m_temp_code->push_instr(Instr_t::mov);
        store_instr->m_arg0    = (u64)Register::rdx;
        store_instr->m_arg1    = (u64)Register::rax;
        store_instr->m_comment = "store result";

        Instr* skip_true_branch = m_temp_code->push_instr(Instr_t::jne);
        skip_true_branch->m_comment = "jump if register is false";

        Instr* skip_false_branch = nullptr;

        if ( auto true_scope = i_cond_struct->get_condition_true_scope() )
        {
            compile_scope(true_scope);

            if ( auto for_loop = _node->as<ForLoopNode>() )
            {
                // insert end-loop instruction.
                compile_member(for_loop->get_iter_expr());

                // insert jump to condition instructions.
                auto loop_jump = m_temp_code->push_instr(Instr_t::jmp);
                loop_jump->m_arg0    = signed_diff(condition_instr_line, loop_jump->m_line);
                loop_jump->m_comment = "jump back to loop begining";

            }
            else if (i_cond_struct->get_condition_false_scope())
            {
                skip_false_branch = m_temp_code->push_instr(Instr_t::jmp);
                skip_false_branch->m_comment = "jump false branch";
            }
        }

        skip_true_branch->m_arg0 = m_temp_code->get_next_index() - skip_true_branch->m_line;

        if ( auto false_scope = i_cond_struct->get_condition_false_scope() )
        {
            compile_scope(false_scope);
            if ( skip_false_branch )
            {
                skip_false_branch->m_arg0 = m_temp_code->get_next_index() - skip_false_branch->m_line;
            }
        }
    }
    else if ( auto instr_node = _node->as<InstructionNode>() )
    {
        const Member* root_member = instr_node->get_root_node_member();
        NODABLE_ASSERT(root_member)
        compile_member(root_member);
    }
    else
    {
        // eval inputs
        for ( const Node* each_input : _node->input_slots().content() )
        {
            if ( !each_input->is<VariableNode>() )
            {
                compile_node(each_input);
            }
        }

        // eval node
        bool should_be_evaluated = _node->has<InvokableComponent>() || _node->is<VariableNode>() || _node->is<LiteralNode>();
        if ( should_be_evaluated )
        {
            Instr *instr     = m_temp_code->push_instr(Instr_t::call);
            instr->m_arg0    = (u64)FctId::eval_node;
            instr->m_arg1    = (u64)_node;
            instr->m_comment =
                    std::string{_node->get_label()} +
                    " (initial value is: " +
                    _node->props()->get(k_value_member_name)->convert_to<std::string>() +
                    ")";
        }
    }

}

void Asm::Compiler::compile_graph_root(Node* _program_graph_root)
{
    m_temp_code = std::make_unique<Code>(_program_graph_root);

    try
    {
        auto scope = _program_graph_root->get<Scope>();
        NODABLE_ASSERT(scope)
        compile_scope(scope, true); // <--- true here is a hack, TODO: implement a real ReturnNode
    }
    catch ( const std::exception& e )
    {
        m_temp_code.reset();
        LOG_ERROR("Compiler", "Unable to create assembly code for program. Reason: %s\n", e.what());
    }
}

std::unique_ptr<const Code> Asm::Compiler::compile(Node* _program_graph)
{
    if ( is_program_valid(_program_graph))
    {
        compile_graph_root(_program_graph);
        LOG_MESSAGE("Compiler", "Program compiled.\n");
        return std::move(m_temp_code);
    }
    return nullptr;
}

std::string Asm::Code::to_string(const Code* _code)
{
    std::string result;

    result.append( "------------<=[ Program begin ]=>------------\n");
    for( Instr* each_instruction : _code->m_instructions )
    {
        result.append( Instr::to_string(*each_instruction) );
        result.append("\n");
    }
    result.append("------------<=[ Program end    ]=>------------\n");
    return result;
}