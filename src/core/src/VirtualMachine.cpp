#include <nodable/core/VirtualMachine.h>

#include <nodable/core/VariableNode.h>
#include <nodable/core/Log.h>
#include <nodable/core/Scope.h>
#include <nodable/core/InvokableComponent.h>
#include "nodable/core/String.h"

using namespace ndbl;
using opcode = ndbl::assembly::opcode_t;

CPU::CPU()
{
    clear_registers();
}

void CPU::clear_registers()
{
    for( size_t id = 0; id < std::size( m_register ); ++id )
    {
        write( (Register)id, qword());
    }
}

qword CPU::read(Register _id)const
{
    LOG_VERBOSE("VM::CPU", "read register %s (value: %s)\n", assembly::to_string(_id), m_register[_id].to_string().c_str() )
    return m_register[_id];
}

qword& CPU::_read(Register _id)
{
    LOG_VERBOSE("VM::CPU", "_read register %s (value: %s)\n", assembly::to_string(_id), m_register[_id].to_string().c_str() )
    return m_register[_id];
}

void CPU::write(Register _id, qword _data)
{
    qword& mem_dst = _read(_id);
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
    qword eip = m_cpu.read(Register::eip);
    eip.u64 += _amount;
    m_cpu.write(Register::eip, eip );
}

void VirtualMachine::run_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    LOG_MESSAGE("VM", "Running program ...\n")
    m_is_program_running = true;

    m_cpu.clear_registers();

    while( is_there_a_next_instr() && get_next_instr()->opcode != opcode::ret )
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

    switch ( next_instr->opcode )
    {
        case opcode::cmp:
        {
            qword left  = m_cpu.read(static_cast<Register>(next_instr->cmp.left.u8));  // dereference registers, get their value
            qword right = m_cpu.read(static_cast<Register>(next_instr->cmp.right.u8));
            qword result;
            result.set<bool>(left.b == right.b);
            m_cpu.write(Register::rax, result);       // boolean comparison
            advance_cursor();
            success = true;
            break;
        }

        case opcode::deref_ptr:
        {
            NODABLE_ASSERT_EX(next_instr->uref.qword_ptr, "in instruction deref_ptr: uref.qword_ptr is nullptr")
            qword qword = *next_instr->uref.qword_ptr;
            m_cpu.write(Register::rax, qword );

            type t = *next_instr->uref.qword_type;
            if( t == type::get<bool>() )
            {
                LOG_VERBOSE("VM", "value dereferenced: %b\n", qword.b);
            }
            else if( t == type::get<double >() )
            {
                LOG_VERBOSE("VM", "value dereferenced: %d\n", qword.d );
            }
            else if( t == type::get<i16_t >() )
            {
                LOG_VERBOSE("VM", "value dereferenced: %i\n", qword.i16 );
            }
            else if( t == type::get<std::string>() )
            {
                LOG_VERBOSE("VM", "pointed string: %s\n", ((std::string*)qword.ptr)->c_str() );
            }
            else if( t == type::get<void*>() )
            {
                LOG_VERBOSE("VM", "pointed address: %s\n", String::fmt_ptr(qword.ptr).c_str() );
            }
            else
            {
                NODABLE_ASSERT_EX(false, "This type is not handled!")
            }

            advance_cursor();
            success = true;
            break;
        }

        case opcode::mov:
        {
            // write( <destination_register>, <source_data>)
            m_cpu.write(static_cast<Register>(next_instr->mov.dst.u8), next_instr->mov.src);

            advance_cursor();
            success = true;
            break;
        }

        case opcode::push_var:
        {
            advance_cursor();
            auto* variable = const_cast<VariableNode*>( next_instr->push.var ); // hack !
            variable->get_value()->get_variant()->ensure_is_initialized(false);
            success = true;
            break;
        }

        case opcode::pop_var:
        {
            advance_cursor();
            auto* variable = const_cast<VariableNode*>( next_instr->push.var ); // hack !
            NODABLE_ASSERT_EX(variable->get_value()->get_variant()->is_initialized(),
                              "Variable should be initialized since it should have been pushed earlier!");
            variable->get_value()->get_variant()->reset_value();
            variable->get_value()->get_variant()->ensure_is_initialized(false);
            success = true;
            break;
        }

        case opcode::push_stack_frame: // ensure variable declared in this scope are unitialized before/after scope
        case opcode::pop_stack_frame:
        {
            advance_cursor();
            success = true;
            break;
        }

        case opcode::eval_node:
        {
            bool transfer_inputs = true;
            auto node            = const_cast<Node*>( next_instr->eval.node ); // hack !

            auto transfer_input_values = [](Node* _node)
            {
                for(Member* each_member : _node->props()->by_id())
                {
                    Member* input = each_member->get_input();

                    if( input
                        && !each_member->is_connected_by_ref()
                        && each_member->get_type() != type::null
                        && input->get_type() != type::null )
                    {
                        *each_member->get_variant() = *input->get_variant();
                    }
                }
            };

            if( auto variable = node->as<VariableNode>())
            {
                variant* variant = variable->get_value()->get_variant();
                if( !variant->is_initialized() )
                {
                    variant->ensure_is_initialized();
                    variant->flag_defined();
                }
                else
                {
                    transfer_inputs = false;
                }
            }

            if( transfer_inputs )
            {
                transfer_input_values(node);
            }

            // evaluate Invokable Component, could be an operator or a function
            if(auto invokable = node->get<InvokableComponent>())
            {
                invokable->update();
            }

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
            qword rax = m_cpu.read(Register::rax);
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
                get_next_instr()->opcode == opcode::eval_node
               && m_last_step_next_instr != get_next_instr();
    };

    bool must_exit = false;

    while(is_there_a_next_instr() && !must_break() && !must_exit )
    {
        must_exit = get_next_instr()->opcode == opcode::ret;
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

        switch ( next_instr->opcode )
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
    const qword& eip = m_cpu.read(Register::eip);
    return eip.u64 < m_program_asm_code->size();
}

qword VirtualMachine::get_last_result()const
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

qword VirtualMachine::read_cpu_register(Register _register)const
{
    return m_cpu.read(_register);
}

const Code *VirtualMachine::get_program_asm_code()
{
    return m_program_asm_code.get();
}
