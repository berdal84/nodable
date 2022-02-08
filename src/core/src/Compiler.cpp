#include <nodable/Compiler.h>
#include <nodable/VariableNode.h>
#include <nodable/Log.h>
#include <nodable/AbstractConditionalStruct.h>
#include <nodable/ForLoopNode.h>
#include <nodable/Scope.h>
#include <nodable/InstructionNode.h>
#include <nodable/LiteralNode.h>
#include <nodable/InvokableComponent.h>

using namespace Nodable;
using namespace Nodable::Asm;

std::string Asm::Instr::to_string(const Instr& _instr)
{
    std::string result;

    // append "<line> :"
    std::string str = std::to_string(_instr.m_line);
    while( str.length() < 4 )
        str.append(" ");
    result.append( str );
    result.append( ": " );

    // append instruction type
    result.append( Nodable::to_string(_instr.m_type) + " " );

    // optionally append parameters
    switch ( _instr.m_type )
    {
        case Instr_t::call:
        {
            FctId fct_id   = (FctId)_instr.m_left_h_arg;
            Member* member = (Member*)_instr.m_right_h_arg;
            result.append( Nodable::to_string(fct_id) );
            result.append( " [" + std::to_string((size_t)member) + "]");
            break;
        }

        case Instr_t::mov:
        case Instr_t::cmp:
        {
            result.append("%" + std::to_string( _instr.m_left_h_arg ) );
            result.append(", %" + std::to_string( _instr.m_right_h_arg ) );
            break;
        }

        case Instr_t::jne:
        case Instr_t::jmp:
        {
            result.append( std::to_string( _instr.m_left_h_arg ) );
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

std::string Nodable::to_string(Instr_t _type)
{
    switch( _type)
    {
        case Instr_t::mov:   return "mov";
        case Instr_t::ret:   return "ret";
        case Instr_t::call:  return "call";
        case Instr_t::jmp:   return "jpm";
        case Instr_t::jne:   return "jne";
        default:             return "???";
    }
}

std::string Nodable::to_string(Register _register)
{
    switch( _register)
    {
        case Register::rax: return "%rax";
        case Register::rdx: return "%rdx";
        default:            return "%???";
    }
}

std::string Nodable::to_string(FctId _id)
{
    switch( _id)
    {
        case FctId::eval_member:      return "eval_member";
        case FctId::eval_node:        return "eval_node";
        case FctId::push:             return "push";
        case FctId::pop_stack_frame:  return "pop_stack_frame";
        case FctId::push_stack_frame: return "push_stack_frame";
        default:                      return "???";
    }
}

Code::~Code()
{
    reset();
}

Instr* Code::push_instr(Instr_t _type)
{
    Instr* instr = new Instr(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

void Code::reset()
{
    for( auto each : m_instructions )
        delete each;
    m_instructions.clear();
    m_instructions.resize(0);
}

bool Asm::Compiler::is_program_valid(const Node* _program)
{
    bool is_valid;

    // check if program an be run
    const VariableNodes& vars = _program->get<Scope>()->get_variables();
    bool found_a_var_uninit = false;
    auto it = vars.begin();
    while(!found_a_var_uninit && it != vars.end() )
    {
        if( !(*it)->isDeclared() )
        {
            LOG_ERROR("Compiler", "Unable to load program because %s is not declared.\n", (*it)->getName() );
            found_a_var_uninit = true;
        }
        ++it;
    }

    is_valid = !found_a_var_uninit;

    return is_valid;
}

void Asm::Compiler::append_to_assembly_code( const Member * _member )
{
    NODABLE_ASSERT(_member);
    {
        if (_member->isType(Reflect::Type_Object_Ptr))
        {
            /*
             * Members can point to a Node*
             */
            append_to_assembly_code( (const Node *)*_member);
        }
        else
        {
            /*
             * if the member has no input it means it is a simple literal value and we have nothing to compute,
             * instead we traverse the syntax tree starting from the node connected to it.
             * Once we have the list of the nodes to be updated, we loop on them.
             */

            Member *input = _member->getInput();
            if ( input )
            {
                append_to_assembly_code( input->getOwner() );
            }
        }

        /* evaluate member */
        {
            Instr *instr = m_output->push_instr(Instr_t::call);
            instr->m_left_h_arg = (i64_t) FctId::eval_member;
            instr->m_right_h_arg = (i64_t) _member;
            instr->m_comment = "eval " + std::string{_member->getOwner()->getLabel()} + " -> " + _member->getName();
        }
    }
}

void Asm::Compiler::append_to_assembly_code(const Node* _node)
{
    NODABLE_ASSERT(_node);

    if (_node->has<Scope>() && _node->get_parent() )
    {
        // call push_stack_frame
        Instr *instr = m_output->push_instr(Instr_t::call);
        instr->m_left_h_arg = (i64_t) FctId::push_stack_frame;
        instr->m_right_h_arg = (i64_t) _node;
        instr->m_comment = std::string{_node->getShortLabel()} + "'s scope";
    }

    if ( auto conditional_struct = _node->as<AbstractConditionalStruct>(); conditional_struct )
    {
        // for_loop init instruction
        if ( auto for_loop = _node->as<ForLoopNode>() )
        {
            append_to_assembly_code( for_loop->get_init_expr() );
        }

        long condition_instr_line = m_output->get_next_pushed_instr_index();

        Member* condition_member = conditional_struct->get_condition();
        NODABLE_ASSERT(condition_member)
        Node* _condition_node = (Node*)*condition_member;
        append_to_assembly_code( _condition_node );

        Instr* store_instr = m_output->push_instr(Instr_t::mov);
        store_instr->m_left_h_arg = Register::rdx;
        store_instr->m_right_h_arg = Register::rax;
        store_instr->m_comment = "store result";

        Instr* skip_true_branch = m_output->push_instr(Instr_t::jne);
        skip_true_branch->m_comment = "jump if register is false";

        Instr* skip_false_branch = nullptr;

        if ( auto true_branch = conditional_struct->get_condition_true_branch() )
        {
            append_to_assembly_code( true_branch->get_owner() );

            if ( auto for_loop = _node->as<ForLoopNode>() )
            {
                // insert end-loop instruction.
                append_to_assembly_code( for_loop->get_iter_expr() );

                // insert jump to condition instructions.
                auto loop_jump = m_output->push_instr(Instr_t::jmp);
                loop_jump->m_left_h_arg = condition_instr_line - loop_jump->m_line;
                loop_jump->m_comment = "jump back to loop begining";

            }
            else if (conditional_struct->get_condition_false_branch())
            {
                skip_false_branch = m_output->push_instr(Instr_t::jmp);
                skip_false_branch->m_comment = "jump false branch";
            }
        }

        skip_true_branch->m_left_h_arg = m_output->get_next_pushed_instr_index() - skip_true_branch->m_line;

        if ( auto false_branch = conditional_struct->get_condition_false_branch() )
        {
            append_to_assembly_code( false_branch->get_owner() );
            skip_false_branch->m_left_h_arg = m_output->get_next_pushed_instr_index() - skip_false_branch->m_line;
        }
    }
    else if (_node->has<Scope>() )
    {
        for( auto each : _node->get_children() )
        {
            append_to_assembly_code(each);
        }
    }
    else if ( auto instr_node = _node->as<InstructionNode>() )
    {
        Member* root_member = instr_node->get_root_node_member();
        NODABLE_ASSERT(root_member)
        Node* root_node = (Node*)*root_member;

        // eval node
        append_to_assembly_code( root_node );
    }
    else
    {
        // eval inputs
        for ( const Node* each_input : _node->getInputs() )
        {
            if ( !each_input->is<VariableNode>() )
                append_to_assembly_code( each_input );
        }
    }

    // eval node
    bool should_be_evaluated =
               _node->has<InvokableComponent>()
            || _node->is<InstructionNode>()
            || _node->is<LiteralNode>()
            || _node->is<VariableNode>();

    if ( should_be_evaluated )
    {
        Instr *instr = m_output->push_instr(Instr_t::call);
        instr->m_left_h_arg = (i64_t) FctId::eval_node;
        instr->m_right_h_arg = (i64_t) _node;
        instr->m_comment = std::string{_node->getLabel()};
    }

    if ( _node->has<Scope>() && _node->get_parent() )
    {
        // call pop_stack_frame
        Instr *instr = m_output->push_instr(Instr_t::call);
        instr->m_left_h_arg = (i64_t) FctId::pop_stack_frame;
        instr->m_right_h_arg = (i64_t) _node;
        instr->m_comment = std::string{_node->getShortLabel()} + "'s scope";
    }
}

Code* Asm::Compiler::create_assembly_code(const Node* _program)
{
    /*
     * Here we take the program's base scope node (a tree) and we flatten it to an
     * instruction list. We add some jump instruction in order to skip portions of code.
     * This works "a little bit" like a compiler, at least for the "tree to list" point of view.
     */
    // delete m_output; we are NOT responsible to delete, m_output point could be in use.
    m_output = new Code();

    try
    {
        append_to_assembly_code(_program);
        m_output->push_instr(Instr_t::ret);
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR("Compiler", "Unable to create assembly code for program.");
    }

    return m_output;
}