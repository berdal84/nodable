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

void CPU::init_eip_register()
{
    /* eip: instruction pointer */
    MemSpace mem(MemSpace::Type::U64);
    mem.data.m_u64 = 0;
    write_register(Register::eip, mem);
};

void CPU::clear_registers()
{
    for( size_t i = 0; i < std::size( m_register ); ++i )
    {
        m_register[i].reset();
    }
    init_eip_register();
}

Asm::MemSpace& CPU::read_register(Register _id)
{
    return m_register[_id];
}

const Asm::MemSpace& CPU::read_register(Register _id) const
{
    return m_register[_id];
}

void CPU::write_register(Register _id, MemSpace _mem_src)
{
    MemSpace& mem_dst = m_register[_id];
    LOG_VERBOSE("VM::CPU", "write_register %s\n", Asm::to_string(_id) )
    LOG_VERBOSE("VM::CPU", " - mem before: %s\n", mem_dst.to_string().c_str() )
    mem_dst = _mem_src;
    LOG_VERBOSE("VM::CPU", " - mem after:  %s\n", mem_dst.to_string().c_str() )
}

VM::VM()
    : m_is_debugging(false)
    , m_is_program_running(false)
    , m_next_node(nullptr)
    , m_program_asm_code(nullptr)
{

}

void VM::advance_cursor(i64 _amount)
{
    MemSpace mem = m_cpu.read_register(Register::eip);
    NODABLE_ASSERT(mem.type == MemSpace::Type::U64);
    mem.data.m_u64 += _amount; // can overflow
    m_cpu.write_register(Register::eip, mem);
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
            MemSpace left  = next_instr->cmp.left;
            MemSpace right = next_instr->cmp.right;

            NODABLE_ASSERT(left.type == MemSpace::Type::Register);
            NODABLE_ASSERT(right.type == MemSpace::Type::Register);

            // dereference registers
            MemSpace *deref_left, *deref_right;

            if ( left.type == MemSpace::Type::Register )
            {
                deref_left = &m_cpu.read_register(left.data.m_register);
            }
            else
            {
                deref_left = &left;
            }

            if ( right.type == MemSpace::Type::Register )
            {
                deref_right = &m_cpu.read_register(right.data.m_register);
            }
            else
            {
                deref_right = &right;
            }

            NODABLE_ASSERT(deref_left->type != Asm::MemSpace::Type::VariantPtr); // we only allow right for pointers

            if ( deref_right->type == Asm::MemSpace::Type::VariantPtr)
            {
                NODABLE_ASSERT( !deref_right->data.m_variant->is_meta_type(R::get_meta_type<Node*>())); // we do not handler Node*
            }

            bool cmp_result;
            switch (deref_left->type)
            {
                case MemSpace::Type::Boolean:
                    if( deref_right->type == Asm::MemSpace::Type::VariantPtr)
                        cmp_result = deref_left->data.m_bool == (bool)*deref_right->data.m_variant;
                    else
                        cmp_result = deref_left->data.m_bool == deref_right->data.m_bool;
                    break;
                case MemSpace::Type::U64:
                    if( deref_right->type == Asm::MemSpace::Type::VariantPtr)
                        cmp_result = deref_left->data.m_u64 == (u64&)*deref_right->data.m_variant;
                    else
                        cmp_result = deref_left->data.m_u64 == deref_right->data.m_u64;
                    break;
                case MemSpace::Type::Double:
                    if( deref_right->type == Asm::MemSpace::Type::VariantPtr)
                        cmp_result = deref_left->data.m_double == (double)*deref_right->data.m_variant;
                    else
                        cmp_result = deref_left->data.m_double == deref_right->data.m_double;
                    break;
                default:
                    NODABLE_ASSERT(false) // TODO
            }

            MemSpace mem(MemSpace::Type::Boolean);
            mem.data.m_bool = cmp_result;
            m_cpu.write_register(Register::rax, mem);
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::mov:
        {
            MemSpace src = next_instr->mov.src;
            MemSpace dst = next_instr->mov.dst;
            MemSpace* deref_src;
            MemSpace* deref_dst;

            NODABLE_ASSERT(src.type != Asm::MemSpace::Type::Undefined)
            NODABLE_ASSERT(dst.type != Asm::MemSpace::Type::Undefined)

            if (dst.type == MemSpace::Type::Register)
            {
                deref_dst = &m_cpu.read_register(dst.data.m_register);
            }
            else
            {
                deref_dst = &dst;
            }

            if (src.type == MemSpace::Type::Register)
            {
                deref_src = &m_cpu.read_register(src.data.m_register);
                NODABLE_ASSERT(src.type != Asm::MemSpace::Type::Undefined)
            }
            else
            {
                deref_src = &src;
            }

            NODABLE_ASSERT(deref_dst->type != MemSpace::Type::Register); // we should point the register´ value
            NODABLE_ASSERT(deref_dst->type != MemSpace::Type::Register); // we should point the register´ value

            *deref_dst = *deref_src;

            NODABLE_ASSERT(deref_dst->type     == deref_src->type)
            NODABLE_ASSERT(deref_dst->data.m_u64 == deref_src->data.m_u64 )

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
            VariableNode* variable = const_cast<VariableNode*>( next_instr->push.variable ); // hack !
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

            // save node value in rax register
            if( node->props()->has(k_value_member_name))
            {
                MemSpace mem_space;
                mem_space.type = Asm::MemSpace::Type::VariantPtr;
                mem_space.data.m_variant = node->props()->get(k_value_member_name)->get_data();
                m_cpu.write_register(Register::rax, mem_space);
            }

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
            MemSpace rax = m_cpu.read_register(Register::rax);
            NODABLE_ASSERT(rax.type == MemSpace::Type::Boolean);
            bool equals = rax.data.m_bool;
            if ( equals )
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
    const MemSpace& eip = m_cpu.read_register(Register::eip);
    return eip.data.m_u64 < m_program_asm_code->size();
}

const MemSpace* VM::get_last_result()const
{
    const MemSpace& rax = m_cpu.read_register(Register::rax);
    if (rax.type == MemSpace::Type::Undefined)
    {
        LOG_WARNING("VM", "get_last_result() will return nullptr.\n")
        return nullptr;
    }
    return &rax;
}

Instr* VM::get_next_instr() const
{
    if ( is_there_a_next_instr() )
    {
        const MemSpace& eip = m_cpu.read_register(Register::eip);
        NODABLE_ASSERT(eip.type == MemSpace::Type::U64);
        return m_program_asm_code->get_instruction_at(eip.data.m_u64);
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
    return m_cpu.read_register(_register);
}

