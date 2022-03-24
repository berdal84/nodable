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
    while( is_there_a_next_instr() && get_next_instr()->type != Instr_t::ret )
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

    LOG_VERBOSE("VM", "processing line %i.\n", (int)next_instr->line );

    switch ( next_instr->type )
    {
        case Instr_t::cmp:
        {
            u64 compare = (i64)read_register(next_instr->cmp.left) - (i64)read_register(next_instr->cmp.left);
            write_register( Register::rax, compare );
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::mov:
        {
            write_register(next_instr->mov.dst, read_register(next_instr->mov.src));
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::push_stack_frame: // do nothing, just mark visually the beginning of a scope.
        {
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::push_var:
        {
            advance_cursor();
            VariableNode* variable = next_instr->push.var;
            if (variable->is_initialized() )
            {
                variable->set_initialized(false);
            }
            // TODO: push variable to the future stack

            success = true;
            break;
        }

        case Instr_t::pop_stack_frame:
        {
            auto scope = next_instr->pop.scope;
            for( VariableNode* each_var : scope->get_variables() )
            {
                if (each_var->is_initialized() )
                {
                    each_var->set_initialized(false);
                }
            }
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::eval_node:
        {
            auto node = next_instr->eval.node;
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

        case Instr_t::store_data:
        {
            const Variant* data = next_instr->store.data;
            write_register(Register::rax, (u64)data); //copy variant address
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::jmp:
        {
            advance_cursor(next_instr->jmp.offset);
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
                advance_cursor(next_instr->jmp.offset);
            }
            success = true;
            break;
        }

        case Instr_t::ret:
            //advance_cursor();
            success = false;
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
        return get_next_instr()->type == Instr_t::ret ||
               get_next_instr()->type == Instr_t::eval_node
               && m_last_step_next_instr != get_next_instr();
    };

    while(is_there_a_next_instr() && !must_break() )
    {
        _stepOver();
    }


    bool continue_execution = is_there_a_next_instr();
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

        switch ( next_instr->type )
        {
            case Instr_t::eval_node:
            {
                m_next_node = next_instr->eval.node;
                break;
            }

            default:
                break;
        }
        LOG_MESSAGE("VM", "Step over (current line %#1llx)\n", next_instr->line)
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

bool VM::is_there_a_next_instr() const
{
    auto next_inst_id = read_register(Register::eip);
    return next_inst_id < m_program_asm_code->size();
}

Instr* VM::get_next_instr() const
{
    if ( is_there_a_next_instr() )
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
