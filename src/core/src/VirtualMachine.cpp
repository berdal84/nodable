#include <nodable/VirtualMachine.h>

#include <nodable/ScopedCodeBlockNode.h>
#include <nodable/GraphTraversal.h>
#include <nodable/VariableNode.h>
#include <nodable/Log.h>
#include <nodable/AbstractConditionalStruct.h>
#include <nodable/ForLoopNode.h>

using namespace Nodable;
using namespace Nodable::Asm;

VirtualMachine::VirtualMachine()
    : m_program_graph(nullptr)
    , m_is_debugging(false)
    , m_is_program_running(false)
    , m_current_node(nullptr)
    , m_program_asm_code(nullptr)
{

}

bool VirtualMachine::load_program(ScopedCodeBlockNode* _program)
{
    Asm::Compiler compiler;

    if ( compiler.is_program_valid(_program) )
    {
        // unload current program
        if (m_program_graph)
        {
            unload_program();
        }

        if ( compiler.create_assembly_code(_program) )
        {
            m_program_graph     = _program;
            m_program_asm_code = compiler.get_output_assembly();

            LOG_MESSAGE("VM", "Program's tree compiled.\n");
            LOG_VERBOSE("VM", "Find bellow the compilation result:\n");
            LOG_VERBOSE("VM", "---- Program begin -----\n");
            Instr* curr = get_current_instruction();
            while( curr )
            {
                LOG_VERBOSE("VM", "%s \n", Instr::to_string(*curr ).c_str() );
                advance_cursor(1);
                curr = get_current_instruction();
            }
            reset_cursor();
            LOG_VERBOSE("VM", "---- Program end -----\n");
            return true;
        }
        else
        {
            LOG_ERROR("VM", "Unable to compile program's tree.\n");
            return false;
        }

    }
    return false;
}

void VirtualMachine::run_program()
{
    NODABLE_ASSERT(m_program_graph != nullptr);
    LOG_VERBOSE("VM", "Running...\n")
    m_is_program_running = true;

    while(!is_program_over())
    {
        _stepOver();
    }
    stop_program();
}

void VirtualMachine::stop_program()
{
    m_is_program_running = false;
    m_is_debugging = false;
    m_current_node = nullptr;
    LOG_VERBOSE("VM", "Stopped.\n")
}

void VirtualMachine::unload_program() {
    // TODO: clear context
    this->m_program_graph = nullptr;
}

bool VirtualMachine::_stepOver()
{
    bool success = false;
    Instr* curr_instr = get_current_instruction();

    LOG_VERBOSE("VM", "processing line %i.\n", (int)curr_instr->m_line );

    switch ( curr_instr->m_type )
    {
        case Instr::mov:
        {
            NODABLE_ASSERT(m_register);
            Register dst_register = mpark::get<Register>(curr_instr->m_left_h_arg );
            Register src_register = mpark::get<Register>(curr_instr->m_right_h_arg );
            m_register[dst_register].set(m_register[src_register].convert_to<bool>() );
            advance_cursor();
            success = true;
            break;
        }

        case Instr::call:
        {
            FctId fct_id = mpark::get<FctId>(curr_instr->m_left_h_arg );

            switch( fct_id )
            {
                case FctId::eval_member:
                {
                    Member *member = mpark::get<Member *>(curr_instr->m_right_h_arg);
                    m_current_node = member->getOwner();

                    /*
                     * if the member has no input it means it is a simple literal value and we have nothing to compute,
                     * instead we traverse the syntax tree starting from the node connected to it.
                     * Once we have the list of the nodes to be updated, we loop on them.
                     * TODO: traverse graph in advance during compilation step.
                     */

                    if (Member *input = member->getInput())
                    {
                        /*
                         * traverse the syntax tree (graph)
                         */
                        TraversalFlag flags = TraversalFlag_FollowInputs | TraversalFlag_FollowNotDirty |
                                              TraversalFlag_AvoidCycles;
                        m_traversal.traverse(input->getOwner(), flags);

                        /*
                         * eval each traversed node
                         */
                        size_t total(m_traversal.getStats().m_traversed.size());
                        size_t idx = 1;
                        for (auto *eachNodeToEval : m_traversal.getStats().m_traversed)
                        {
                            eachNodeToEval->eval();
                            eachNodeToEval->setDirty(false);
                            LOG_VERBOSE("VM", "Eval (%i/%i): \"%s\" (class %s) \n", idx, (int) total,
                                        eachNodeToEval->getLabel(), eachNodeToEval->get_class()->get_name())
                            idx++;
                        }
                    }

                    m_register[rax] = *member->getData(); // store result.
                    advance_cursor();
                    success = true;
                }
            }

            break;
        }

        case Instr::jmp:
        {
            advance_cursor(mpark::get<long>(curr_instr->m_left_h_arg));
            success = true;
            break;
        }

        case Instr::jne:
        {
            if ( m_register[rdx] )
            {
                advance_cursor();
            }
            else
            {
                advance_cursor(mpark::get<long>(curr_instr->m_left_h_arg));
            }
            success = true;
            break;
        }

        case Instr::ret:
            success = true;
            break;

        default:
            success = false;
    }

    return success;
}

bool VirtualMachine::step_over()
{
    bool _break = false;
    while( !is_program_over() && !_break )
    {
        _break = get_current_instruction()->m_type == Instr::call;
        _stepOver();
    }

    bool continue_execution = !is_program_over();
    if( !continue_execution )
    {
        stop_program();
        m_current_node = nullptr;
    }
    return continue_execution;
}

void VirtualMachine::debug_program()
{
    NODABLE_ASSERT(this->m_program_graph != nullptr);
    m_is_debugging = true;
    m_is_program_running = true;
    reset_cursor();
    m_current_node = m_program_graph;
}
