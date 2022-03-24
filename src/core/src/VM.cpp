#include <nodable/core/VM.h>

#include <nodable/core/VariableNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/Scope.h>

using namespace Nodable;
using namespace Nodable::Asm;

VM::VM()
    : m_is_debugging(false)
    , m_is_program_running(false)
    , m_next_node(nullptr)
    , m_program_asm_code(nullptr)
{

}

void VM::clear_registers()
{
    for( size_t i = 0; i < std::size( m_register ); ++i )
    {
        m_register[i] = 0;
    }
}
void VM::run_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    LOG_MESSAGE("VM", "Running program ...\n")
    m_is_program_running = true;

    reset_cursor();
    clear_registers();
    while(!is_program_over())
    {
        _stepOver();
    }
    stop_program();
    LOG_MESSAGE("VM", "Program terminated\n")
}

void VM::stop_program()
{
    m_is_program_running = false;
    m_is_debugging = false;
    m_next_node = nullptr;
    LOG_MESSAGE("VM", "Program stopped\n")
}

void VM::release_program()
{
    m_program_asm_code.reset();
    reset_cursor();
    clear_registers();
    stop_program();
}

bool VM::_stepOver()
{
    bool success;
    Instr* next_instr = get_next_instr();

    LOG_VERBOSE("VM", "processing line %i.\n", (int)next_instr->m_line );

    switch ( next_instr->m_type )
    {
//        case Instr::cmp:
//        {
//
//            auto dst_register = (Register)next_instr->m_left_h_arg;
//            auto src_register = (Register)next_instr->m_right_h_arg;
//            m_register[Register::rax] = m_register[dst_register] - m_register[src_register];
//            advance_cursor();
//            success = true;
//            break;
//        }

        case Instr_t::mov:
        {
            auto dst_register = (Register)next_instr->m_arg0;
            auto src_register = (Register)next_instr->m_arg1;
            write_register(dst_register, read_register(src_register));
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::call:
        {
            auto fct_id = (FctId)next_instr->m_arg0;

            switch( fct_id )
            {
                case FctId::push_frame: // do nothing, just mark visually the beginning of a scope.
                {
                    advance_cursor();
                    success = true;
                    break;
                }

                case FctId::push_variable:
                {
                    advance_cursor();
                    auto variable = ((Node*)next_instr->m_arg1)->as<VariableNode>();
                    if (!variable->is_defined() )
                    {
                        variable->set_defined(true);
                    }
                    success = true;
                    break;
                }

                case FctId::pop_frame:
                {
                    auto scope = ((Node *) next_instr->m_arg1)->get<Scope>();
                    for( VariableNode* each_var : scope->get_variables() )
                    {
                        if (each_var->is_defined() )
                        {
                            each_var->set_defined(false);
                        }
                    }
                    advance_cursor();
                    success = true;
                    break;
                }

                case FctId::eval_node:
                {
                    auto node = (Node*)next_instr->m_arg1;
                    node->eval();
                    node->set_dirty(false);

                    // Store a value if exists
                    if (node->props()->has(k_value_member_name) )
                    {
                        Member* member = node->props()->get(k_value_member_name);
                        write_register(Register::rax, (u64) member->get_data()); // copy variant address
                    }

                    advance_cursor();
                    success = true;
                    break;
                }

                case FctId::eval_member:
                {
                    auto member = (Member*)next_instr->m_arg1;
                    write_register(Register::rax, (u64) member->get_data()); //copy variant address
                    advance_cursor();
                    success = true;
                    break;
                }
            }

            break;
        }

        case Instr_t::jmp:
        {
            advance_cursor(next_instr->m_arg0);
            success = true;
            break;
        }

        case Instr_t::jne:
        {
            Variant* variant = (Variant*)(read_register(Register::rax));
            if ( variant->convert_to<bool>() )
            {
                advance_cursor();
            }
            else
            {
                advance_cursor(next_instr->m_arg0);
            }
            success = true;
            break;
        }

        case Instr_t::ret:
            advance_cursor();
            success = true;
            break;

        default:
            advance_cursor();
            success = false;
    }

    return success;
}

bool VM::step_over()
{
    auto must_break = [&]() -> bool {
        return get_next_instr()->m_type == Instr_t::call
               && m_last_step_next_instr != get_next_instr();
    };

    while( !is_program_over() && !must_break() )
    {
        _stepOver();
    }


    bool continue_execution = !is_program_over();
    if( !continue_execution )
    {
        stop_program();
        m_next_node = nullptr;
        m_last_step_next_instr = nullptr;
    }
    else
    {
        // update m_current_node and m_last_step_instr
        m_last_step_next_instr = get_next_instr();
        auto next_instr = get_next_instr();
        if ( next_instr->m_type == Instr_t::call )
        {
            switch ( (FctId)(next_instr->m_arg0) )
            {
                case FctId::eval_member:
                {
                    auto member = (Member *)next_instr->m_arg1;
                    m_next_node = member->get_owner();
                    break;
                }

                case FctId::eval_node:
                {
                    m_next_node = (Node*)next_instr->m_arg1;
                    break;
                }

                default:
                    break;
            }
        }
        LOG_MESSAGE("VM", "Step over (current line %#1llx)\n", next_instr->m_line)
    }

    return continue_execution;
}

void VM::debug_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    m_is_debugging = true;
    m_is_program_running = true;
    reset_cursor();
    clear_registers();
    m_next_node = m_program_asm_code->get_meta_data().root_node;
    LOG_MESSAGE("VM", "Debugging program ...\n")
}

bool VM::is_program_over() const
{
    auto next_inst_id = read_register(Register::eip);
    return next_inst_id >= m_program_asm_code->size();
}

Instr* VM::get_next_instr() const
{
    if ( !is_program_over() )
    {
        auto next_inst_id = read_register(Register::eip);
        return m_program_asm_code->get_instruction_at(next_inst_id);
    }
    return nullptr;
}

bool VM::load_program(std::unique_ptr<const Code> _code)
{
    NODABLE_ASSERT(!m_is_program_running)   // dev must stop before to load program.
    NODABLE_ASSERT(!m_program_asm_code)     // dev must unload before to load.

    m_program_asm_code = std::move(_code);

    return m_program_asm_code && m_program_asm_code->size() != 0;
}
