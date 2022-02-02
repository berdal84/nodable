#include <nodable/Assembly.h>
#include <nodable/VariableNode.h>
#include <nodable/Log.h>
#include <nodable/AbstractConditionalStruct.h>
#include <nodable/ForLoopNode.h>
#include <nodable/Scope.h>

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
    for( auto each : m_instructions )
        delete each;
    m_instructions.clear();
}

Instr* Code::push_instr(Instr_t _type)
{
    Instr* instr = new Instr(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
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

Code* Asm::Compiler::get_output_assembly()
{
    return m_output;
}

void Asm::Compiler::append_to_assembly_code(const Member *_member)
{
    if ( _member ) {
        /*
         * if the member has no input it means it is a simple literal value and we have nothing to compute,
         * instead we traverse the syntax tree starting from the node connected to it.
         * Once we have the list of the nodes to be updated, we loop on them.
         * TODO: traverse graph in advance during compilation step.
         */

        if (Member *input = _member->getInput()) {
            /*
             * traverse the syntax tree (graph)
             */
            TraversalFlag flags =
                    TraversalFlag_FollowInputs
                    | TraversalFlag_FollowNotDirty // eval all
                    | TraversalFlag_AvoidCycles;   // but avoid loops caused by references.

            m_traversal.traverse(input->getOwner(), flags);

            /*
             * eval each traversed node
             */
            size_t idx = 1;
            for (auto *each_node_to_eval : m_traversal.getStats().m_traversed) {
                Instr *instr = m_output->push_instr(Instr_t::call);
                instr->m_left_h_arg = (i64_t) FctId::eval_node;
                instr->m_right_h_arg = (i64_t) each_node_to_eval;
                instr->m_comment = "eval " + std::string{each_node_to_eval->getLabel()};
                idx++;
            }
        }

        {
            Instr *instr = m_output->push_instr(Instr_t::call);
            instr->m_left_h_arg = (i64_t) FctId::eval_node;
            instr->m_right_h_arg = (i64_t) _member->getOwner();
            instr->m_comment = "eval " + std::string{_member->getOwner()->getLabel()};
        }

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
    if ( _node )
    {
        if (_node->has<Scope>() )
        {
            // push_stack_frame
            Instr *instr = m_output->push_instr(Instr_t::call);
            instr->m_left_h_arg = (i64_t) FctId::push_stack_frame;
            instr->m_right_h_arg = (i64_t) _node;
            instr->m_comment = "begin scope";
        }

        if ( _node->get_class()->is<AbstractConditionalStruct>() )
        {
            auto cond = _node->as<AbstractConditionalStruct>();

            // for_loop init instruction
            if ( auto for_loop = _node->as<ForLoopNode>() )
            {
                append_to_assembly_code( for_loop->get_init_expr() );
            }

            long condition_instr_line = get_output_assembly()->get_next_pushed_instr_index();
            append_to_assembly_code( cond->get_condition() );

            Instr* store_instr = m_output->push_instr(Instr_t::mov);
            store_instr->m_left_h_arg = Register::rdx;
            store_instr->m_right_h_arg = Register::rax;
            store_instr->m_comment = "copy last result to a data register.";

            Instr* skip_true_branch = m_output->push_instr(Instr_t::jne);
            skip_true_branch->m_comment = "jump if register is false";

            Instr* skip_false_branch = nullptr;

            if ( auto true_branch = cond->get_condition_true_branch() )
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
                else if (cond->get_condition_false_branch())
                {
                    skip_false_branch = m_output->push_instr(Instr_t::jmp);
                    skip_false_branch->m_comment = "jump false branch";
                }
            }

            skip_true_branch->m_left_h_arg = m_output->get_next_pushed_instr_index() - skip_true_branch->m_line;

            if ( auto false_branch = cond->get_condition_false_branch() )
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
        else
        {
            append_to_assembly_code( _node->getProps()->get("value") );
        }

        if (_node->has<Scope>() )
        {
            // pop_stack_frame
            {
                Instr *instr = m_output->push_instr(Instr_t::call);
                instr->m_left_h_arg = (i64_t) FctId::pop_stack_frame;
                instr->m_right_h_arg = (i64_t) _node;
                instr->m_comment = "end scope";
            }
        }
    }
}

bool Asm::Compiler::create_assembly_code(const Node* _program)
{
    /*
     * Here we take the program's base scope node (a tree) and we flatten it to an
     * instruction list. We add some jump instruction in order to skip portions of code.
     * This works "a little bit" like a compiler, at least for the "tree to list" point of view.
     */
    delete m_output;
    m_output = new Code();

    try
    {
        append_to_assembly_code(_program);
        m_output->push_instr(Instr_t::ret);
    }
    catch ( const std::exception& e )
    {
        LOG_ERROR("Compiler", "Unable to create assembly code for program.");
        m_output = nullptr;
    }

    return m_output != nullptr;
}

