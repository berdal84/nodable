#include <nodable/core/VM.h>

#include <nodable/core/VariableNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/Scope.h>

using namespace Nodable;
using namespace Nodable::Asm;

CPU::CPU()
{
    clear_registers();
}

void CPU::init_eip()
{
    write(Register::eip, 0ull);
};

void CPU::clear_registers()
{
    for( size_t i = 0; i < std::size( m_register ); ++i )
    {
        m_register[i].reset();
    }
    init_eip();
}

Asm::MemSpace CPU::read(Register _id)
{
    return m_register[_id];
}

const Asm::MemSpace& CPU::read(Register _id) const
{
    return m_register[_id];
}

void CPU::write(Register _id, MemSpace _data)
{
    MemSpace& mem_dst = m_register[_id];
    LOG_VERBOSE("VM::CPU", "write_register %s\n", Asm::to_string(_id) )
    LOG_VERBOSE("VM::CPU", " - mem before: %s\n", mem_dst.to_string().c_str() )
    mem_dst = _data;
    LOG_VERBOSE("VM::CPU", " - mem after:  %s\n", mem_dst.to_string().c_str() )
}


VM::VM()
    : m_is_debugging(false)
    , m_is_program_running(false)
    , m_next_node(nullptr)
    , m_program_asm_code(nullptr)
{

}

void VM::advance_cursor(i64_t _amount)
{
    MemSpace mem = m_cpu.read(Register::eip);
    mem.u64 += _amount; // can overflow
    m_cpu.write(Register::eip, mem);
}

void VM::run_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    LOG_MESSAGE("VM", "Running program ...\n")
    m_is_program_running = true;

    m_cpu.clear_registers();

    while( is_there_a_next_instr() && get_next_instr()->type != Instr_t::ret )
    {
        _stepOver();
    }
    stop_program();
    LOG_MESSAGE("VM", "Program terminated\n")
}

void VM::stop_program()
{
    if ( m_is_program_running )
    {
        m_is_program_running = false;
        m_is_debugging       = false;
        m_next_node          = nullptr;
        LOG_MESSAGE("VM", "Program stopped\n")
    }
    else
    {
        LOG_ERROR("VM", "stop_program() failed, program is not running\n")
    }
}

std::unique_ptr<const Code> VM::release_program()
{
    if( m_is_program_running )
    {
        LOG_VERBOSE("VM", "stopping program before continue\n");
        stop_program();
    }

    m_cpu.clear_registers(); // will also clear reset instruction pointer (stored in a register Register::eip)
    LOG_VERBOSE("VM", "registers cleared\n")

    LOG_VERBOSE("VM", "program released\n")
    return std::move(m_program_asm_code);
}

bool VM::_stepOver()
{
    bool success;
    Instr* next_instr = get_next_instr();

    LOG_MESSAGE("VM", "%s\n", Instr::to_string(*next_instr).c_str() );

    switch ( next_instr->type )
    {
        case Instr_t::cmp:
        {
            MemSpace left  = m_cpu.read(next_instr->cmp.left.r);  // dereference registers, get their value
            MemSpace right = m_cpu.read(next_instr->cmp.right.r);
            m_cpu.write(Register::rax, left.b == right.b);       // boolean comparison
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::deref_ptr:
        {
            switch( next_instr->deref.ptr_t )
            {
                case R::Type::Boolean:
                    m_cpu.write(Register::rax, *(bool*)next_instr->deref.ptr );
                    LOG_VERBOSE("VM", "value dereferenced: %b\n", *(bool*)next_instr->deref.ptr );
                    break;
                case R::Type::Double:
                    m_cpu.write(Register::rax, *(double*)next_instr->deref.ptr );
                    LOG_VERBOSE("VM", "value dereferenced: %d\n", *(double*)next_instr->deref.ptr  );
                    break;
                case R::Type::String:
                    m_cpu.write(Register::rax, next_instr->deref.ptr->m_std_string_ptr->c_str() );
                    LOG_VERBOSE("VM", "value dereferenced: %s\n", next_instr->deref.ptr->m_std_string_ptr->c_str() );
                    break;
                case R::Type::Void:
                case R::Type::Class:
                case R::Type::Null:
                    m_cpu.write(Register::rax, next_instr->deref.ptr );
                    break;
            }


            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::mov:
        {
            m_cpu.write(next_instr->mov.dst.r, next_instr->mov.src);      // write source to destination register

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
            auto* variable = const_cast<VariableNode*>( next_instr->push.var ); // hack !
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
            auto node = const_cast<Node*>( next_instr->eval.node ); // hack !
            node->eval();
            node->set_dirty(false);
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
            MemSpace rax = m_cpu.read(Register::rax);
            if ( rax.b )
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
        return
               get_next_instr()->type == Instr_t::eval_node
               && m_last_step_next_instr != get_next_instr();
    };

    bool must_exit = false;

    while(is_there_a_next_instr() && !must_break() && !must_exit )
    {
        must_exit = get_next_instr()->type == Instr_t::ret;
        _stepOver();
    }


    bool continue_execution = is_there_a_next_instr() && !must_exit;
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
    m_cpu.clear_registers();
    m_next_node = m_program_asm_code->get_meta_data().root_node;
    LOG_MESSAGE("VM", "Debugging program ...\n")
}

bool VM::is_there_a_next_instr() const
{
    const MemSpace& eip = m_cpu.read(Register::eip);
    return eip.u64 < m_program_asm_code->size();
}

MemSpace VM::get_last_result()const
{
    return m_cpu.read(Register::rax);
}

Instr* VM::get_next_instr() const
{
    if ( is_there_a_next_instr() )
    {
        return m_program_asm_code->get_instruction_at(m_cpu.read(Register::eip).u64 );
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

const MemSpace& VM::read_cpu_register(Register _register)const
{
    return m_cpu.read(_register);
}

