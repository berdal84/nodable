#include <nodable/core/VirtualMachine.h>

#include <nodable/core/VariableNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/Scope.h>

using namespace Nodable;
using opcode = Nodable::assembly::opcode;

CPU::CPU()
{
    clear_registers();
}

void CPU::init_eip()
{
    write(Register::eip, QWord(0ull));
};

void CPU::clear_registers()
{
    for( size_t id = 0; id < std::size( m_register ); ++id )
    {
        write( (Register)id, QWord());
    }
    init_eip();
}

QWord CPU::read(Register _id)const
{
    LOG_VERBOSE("VM::CPU", "read register %s (value: %s)\n", assembly::to_string(_id), m_register[_id].to_string().c_str() )
    return m_register[_id];
}

QWord& CPU::_read(Register _id)
{
    LOG_VERBOSE("VM::CPU", "_read register %s (value: %s)\n", assembly::to_string(_id), m_register[_id].to_string().c_str() )
    return m_register[_id];
}

void CPU::write(Register _id, QWord _data)
{
    QWord& mem_dst = _read(_id);
    mem_dst = _data;
    LOG_VERBOSE("VM::CPU", "write register %s (value: %s)\n", assembly::to_string(_id), mem_dst.to_string().c_str())
}


VirtualMachine::VirtualMachine()
    : m_is_debugging(false)
    , m_is_program_running(false)
    , m_next_node(nullptr)
    , m_program_asm_code(nullptr)
{

}

void VirtualMachine::advance_cursor(i64_t _amount)
{
    QWord eip = m_cpu.read(Register::eip);
    eip.u64 += _amount;
    m_cpu.write(Register::eip, eip );
}

void VirtualMachine::run_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    LOG_MESSAGE("VM", "Running program ...\n")
    m_is_program_running = true;

    m_cpu.clear_registers();

    while( is_there_a_next_instr() && get_next_instr()->type != opcode::ret )
    {
        _stepOver();
    }
    stop_program();
    LOG_MESSAGE("VM", "Program terminated\n")
}

void VirtualMachine::stop_program()
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

std::unique_ptr<const Code> VirtualMachine::release_program()
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

bool VirtualMachine::_stepOver()
{
    bool success;
    Instruction* next_instr = get_next_instr();

    LOG_MESSAGE("VM", "%s\n", Instruction::to_string(*next_instr).c_str() );

    switch ( next_instr->type )
    {
        case opcode::cmp:
        {
            QWord left  = m_cpu.read(next_instr->cmp.left.r);  // dereference registers, get their value
            QWord right = m_cpu.read(next_instr->cmp.right.r);
            QWord result(left.b == right.b);
            m_cpu.write(Register::rax, result);       // boolean comparison
            advance_cursor();
            success = true;
            break;
        }

        case opcode::deref_ptr:
        {
            QWord qword = *next_instr->uref.qword_ptr;
            m_cpu.write(Register::rax, qword );

            switch( next_instr->uref.qword_type )
            {
                case R::Type::bool_t:
                {
                    LOG_VERBOSE("VM", "value dereferenced: %b\n", qword.b);
                    break;
                }

                case R::Type::double_t:
                {
                    LOG_VERBOSE("VM", "value dereferenced: %d\n", qword.d );
                    break;
                }

                case R::Type::i16_t:
                {
                    LOG_VERBOSE("VM", "value dereferenced: %i\n", qword.i16 );
                    break;
                }

                case R::Type::string_t:
                {
                    LOG_VERBOSE("VM", "pointed string: %s\n", ((std::string*)qword.ptr)->c_str() );
                    break;
                }
                default: NODABLE_ASSERT(false) // not handled
            }


            advance_cursor();
            success = true;
            break;
        }

        case opcode::mov:
        {
            m_cpu.write(next_instr->mov.dst.r, next_instr->mov.src);      // write source to destination register

            advance_cursor();
            success = true;
            break;
        }

        case opcode::push_stack_frame: // do nothing, just mark visually the beginning of a scope.
        {
            advance_cursor();
            success = true;
            break;
        }

        case opcode::push_var:
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

        case opcode::pop_stack_frame:
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

        case opcode::eval_node:
        {
            auto node = const_cast<Node*>( next_instr->eval.node ); // hack !
            node->eval();
            node->set_dirty(false);
            advance_cursor();
            success = true;
            break;
        }

        case opcode::jmp:
        {
            advance_cursor(next_instr->jmp.offset);
            success = true;
            break;
        }

        case opcode::jne:
        {
            QWord rax = m_cpu.read(Register::rax);
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

        case opcode::ret:
            //advance_cursor();
            success = false;
            break;

        default:
            advance_cursor();
            success = false;
    }

    return success;
}

bool VirtualMachine::step_over()
{
    auto must_break = [&]() -> bool {
        return
                get_next_instr()->type == opcode::eval_node
               && m_last_step_next_instr != get_next_instr();
    };

    bool must_exit = false;

    while(is_there_a_next_instr() && !must_break() && !must_exit )
    {
        must_exit = get_next_instr()->type == opcode::ret;
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
            case opcode::eval_node:
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

void VirtualMachine::debug_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    m_is_debugging = true;
    m_is_program_running = true;
    m_cpu.clear_registers();
    m_next_node = m_program_asm_code->get_meta_data().root_node;
    LOG_MESSAGE("VM", "Debugging program ...\n")
}

bool VirtualMachine::is_there_a_next_instr() const
{
    const QWord& eip = m_cpu.read(Register::eip);
    return eip.u64 < m_program_asm_code->size();
}

QWord VirtualMachine::get_last_result()const
{
    return m_cpu.read(Register::rax);
}

Instruction* VirtualMachine::get_next_instr() const
{
    if ( is_there_a_next_instr() )
    {
        return m_program_asm_code->get_instruction_at(m_cpu.read(Register::eip).u64 );
    }
    return nullptr;
}

bool VirtualMachine::load_program(std::unique_ptr<const Code> _code)
{
    NODABLE_ASSERT(!m_is_program_running)   // dev must stop before to load program.
    NODABLE_ASSERT(!m_program_asm_code)     // dev must unload before to load.

    m_program_asm_code = std::move(_code);

    return m_program_asm_code && m_program_asm_code->size() != 0;
}

QWord VirtualMachine::read_cpu_register(Register _register)const
{
    return m_cpu.read(_register);
}

const Code *VirtualMachine::get_program_asm_code()
{
    return m_program_asm_code.get();
}
