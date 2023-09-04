#include "VirtualMachine.h"

#include <string>

#include "fw/core/log.h"
#include "fw/core/format.h"

#include "Graph.h"
#include "InvokableComponent.h"
#include "Scope.h"
#include "VariableNode.h"
#include "core/algorithm.h"

using namespace ndbl;
using namespace fw;
using opcode = ndbl::assembly::Instruction_t;

CPU::CPU()
{
    clear_registers();
}

void CPU::clear_registers()
{
    for( size_t id = 0; id < std::size( m_register ); ++id )
    {
        write( (Register)id, fw::qword());
    }
}

fw::qword CPU::read(Register _id)const
{
    LOG_VERBOSE("VM::CPU", "read register %s (value: %s)\n", assembly::to_string(_id), m_register[_id].to_string().c_str() )
    return m_register[_id];
}

fw::qword& CPU::read_write(Register _id)
{
    LOG_VERBOSE("VM::CPU", "read_write register %s (value: %s)\n", assembly::to_string(_id), m_register[_id].to_string().c_str() )
    return m_register[_id];
}

void CPU::write(Register _id, fw::qword _data)
{
    fw::qword& mem_dst = read_write(_id);
    mem_dst = _data;
    LOG_VERBOSE("VM::CPU", "write register %s (value: %s)\n", assembly::to_string(_id), mem_dst.to_string().c_str())
}


VirtualMachine::VirtualMachine()
    : m_is_debugging(false)
    , m_is_program_running(false)
    , m_program_asm_code(nullptr)
{

}

void VirtualMachine::advance_cursor(i64_t _amount)
{
    fw::qword eip = m_cpu.read(Register::eip);
    eip.u64 += _amount;
    m_cpu.write(Register::eip, eip );
}

void VirtualMachine::run_program()
{
    FW_ASSERT(m_program_asm_code);
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
        m_next_node.reset();
        LOG_MESSAGE("VM", "Program stopped\n")
    }
    else
    {
        LOG_ERROR("VM", "stop_program() failed, program is not running\n")
    }
}

const Code* VirtualMachine::release_program()
{
    if( m_is_program_running )
    {
        LOG_VERBOSE("VM", "stopping program before continue\n");
        stop_program();
    }

    m_cpu.clear_registers(); // will also clear reset instruction pointer (stored in a register Register::eip)
    LOG_VERBOSE("VM", "registers cleared\n")

    LOG_VERBOSE("VM", "program released\n")
    const Code* copy = m_program_asm_code;
    m_program_asm_code = nullptr;
    return copy;
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
            fw::qword left  = m_cpu.read(static_cast<Register>(next_instr->cmp.left.u8));  // dereference registers, get their value
            fw::qword right = m_cpu.read(static_cast<Register>(next_instr->cmp.right.u8));
            fw::qword result;
            result.set<bool>(left.b == right.b);
            m_cpu.write(Register::rax, result);       // boolean comparison
            advance_cursor();
            success = true;
            break;
        }

        case opcode::deref_pool_id:
        {
            fw::qword qword = next_instr->uref.pool_id;
            m_cpu.write(Register::rax, qword );

            const type* ptr_type = next_instr->uref.type;
            if(ptr_type->is<bool>() )
            {
                LOG_VERBOSE("VM", "bool dereferenced: %b\n", qword.b);
            }
            else if(ptr_type->is<double>() )
            {
                LOG_VERBOSE("VM", "double dereferenced: %d\n", qword.d );
            }
            else if(ptr_type->is<i16_t>() )
            {
                LOG_VERBOSE("VM", "i16_t dereferenced: %i\n", qword.i16 );
            }
            else if(ptr_type->is<ID<Node>>() )
            {
                LOG_VERBOSE("VM", "ID<Node> dereferenced: %i\n", qword.i32 );
            }
            else if(ptr_type->is<std::string>() )
            {
                LOG_VERBOSE("VM", "pointed string (%p): %s\n", ((std::string*)qword)->c_str() );
            }
            else if(ptr_type->is<void *>() )
            {
                LOG_VERBOSE("VM", "pointed address: %p\n", qword.ptr );
            }
            else
            {
                FW_EXPECT(false, "This type is not handled!")
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
            next_instr->push.var->property()->ensure_is_initialized(false);
            success = true;
            break;
        }

        case opcode::pop_var:
        {
            advance_cursor();
            VariableNode* variable = next_instr->push.var.get();
            FW_EXPECT(variable->value()->is_initialized(), "Variable should be initialized since it should have been pushed earlier!");
            variable->value()->reset_value();
            variable->value()->ensure_is_initialized(false);
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
            Node* node     = next_instr->eval.node.get();

            auto transfer_input_values = [](Node* _node)
            {
                std::vector<Edge> input_edge = _node->filter_edges(Relation::WRITE_READ);

                // Copy by value (exclude references)
                for(const Edge& edge : _node->edges() )
                {
                    Property* head_property = edge.head.get_property();
                    Property* tail_property = edge.tail.get_property();

                    if(    !head_property->is_ref()
                        && !head_property->is_type_null()
                        && !tail_property->is_type_null() )
                    {
                        *head_property->value() = *tail_property->value();
                    }
                }
            };

            if( auto variable = fw::cast<VariableNode>(node))
            {
                variant* variant = variable->value();
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
            if( InvokableComponent* invokable = node->get_component<InvokableComponent>().get() )
            {
                invokable->update();
            }

            node->dirty = false;
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
            fw::qword rax = m_cpu.read(Register::rax);
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
        m_next_node.reset();
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
    FW_ASSERT(m_program_asm_code);
    m_is_debugging = true;
    m_is_program_running = true;
    m_cpu.clear_registers();
    m_next_node = m_program_asm_code->get_meta_data().root_node;
    LOG_MESSAGE("VM", "Debugging program ...\n")
}

bool VirtualMachine::is_there_a_next_instr() const
{
    const fw::qword& eip = m_cpu.read(Register::eip);
    return eip.u64 < m_program_asm_code->size();
}

fw::qword VirtualMachine::get_last_result()const
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

bool VirtualMachine::load_program(const Code *_code)
{
    FW_ASSERT(!m_is_program_running)   // dev must stop before to load program.
    FW_ASSERT(!m_program_asm_code)     // dev must unload before to load.

    m_program_asm_code = _code;

    return m_program_asm_code && m_program_asm_code->size() != 0;
}

fw::qword VirtualMachine::read_cpu_register(Register _register)const
{
    return m_cpu.read(_register);
}

const Code *VirtualMachine::get_program_asm_code()
{
    return m_program_asm_code;
}

