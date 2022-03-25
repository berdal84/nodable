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

void VM::advance_cursor(i64 _amount)
{
    NODABLE_ASSERT(m_register[Register::eip].type == MemSpace::Type::U64);
    m_register[Register::eip].data.u64 += _amount; // can overflow
}

void VM::init_instruction_pointer()
{
    m_register[Register::eip].type = MemSpace::Type::U64;
    m_register[Register::eip].data.u64 = 0;
};

void VM::clear_registers()
{
    for( size_t i = 0; i < std::size( m_register ); ++i )
    {
        m_register[i].reset();
    }
}
void VM::run_program()
{
    NODABLE_ASSERT(m_program_asm_code);
    LOG_MESSAGE("VM", "Running program ...\n")
    m_is_program_running = true;

    clear_registers();
    init_instruction_pointer(); // uses register, need to be done after clear.

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
    clear_registers(); // will also clear reset instruction pointer (stored in a register Register::eip)
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
            MemSpace left  = next_instr->cmp.left;
            MemSpace right = next_instr->cmp.right;

            NODABLE_ASSERT(left.type == right.type);
            NODABLE_ASSERT(left.type != MemSpace::Type::Register);

            bool cmp_result;
            switch (left.type)
            {
                case MemSpace::Type::Boolean:
                    cmp_result = left.data.b == right.data.b;
                    break;
                case MemSpace::Type::Double:
                    cmp_result = left.data.d == right.data.d;
                    break;
                case MemSpace::Type::U64:
                    cmp_result = left.data.u64 == right.data.u64;
                    break;
                default:
                    NODABLE_ASSERT(false) // TODO
            }
            m_register[Register::rax].type   = MemSpace::Type::Boolean;
            m_register[Register::rax].data.b = cmp_result;
            advance_cursor();
            success = true;
            break;
        }

        case Instr_t::mov:
        {
            MemSpace dst  = next_instr->mov.dst;
            MemSpace src = next_instr->mov.src;

            if (dst.type == MemSpace::Type::Register)
            {
                dst = m_register[dst.data.regid];
            }

            if (src.type == MemSpace::Type::Register)
            {
                src = m_register[src.data.regid];
            }

            NODABLE_ASSERT(dst.type != MemSpace::Type::Register); // we should point the register´ value
            NODABLE_ASSERT(src.type != MemSpace::Type::Register); // we should point the register´ value

            dst = src;

            NODABLE_ASSERT(dst.type     == src.type)
            NODABLE_ASSERT(dst.data.u64 == src.data.u64 )

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
            NODABLE_ASSERT(m_register[Register::rax].type == MemSpace::Type::VariantPtr);
            Variant* variant = m_register[Register::rax].data.variant;
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
    clear_registers();
    init_instruction_pointer(); // uses register, need to be done after clear.
    m_next_node = m_program_asm_code->get_meta_data().root_node;
    LOG_MESSAGE("VM", "Debugging program ...\n")
}

bool VM::is_there_a_next_instr() const
{
    NODABLE_ASSERT(m_register[Register::eip].type == MemSpace::Type::U64);
    auto next_inst_id = m_register[Register::eip].data.u64;
    return next_inst_id < m_program_asm_code->size();
}

const MemSpace* VM::get_last_result()const
{
    if (m_register[Register::rax].type == MemSpace::Type::Undefined)
    {
        LOG_WARNING("VM", "get_last_result() will return nullptr.\n")
        return nullptr;
    }
    return &m_register[Register::rax];
}

Instr* VM::get_next_instr() const
{
    if ( is_there_a_next_instr() )
    {
        NODABLE_ASSERT(m_register[Register::eip].type == MemSpace::Type::U64);
        auto next_inst_id = m_register[Register::eip].data.u64;
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
