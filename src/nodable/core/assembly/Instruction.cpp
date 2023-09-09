#include "Instruction.h"
#include <string>
#include "fw/core/format.h"
#include "Register.h"

using namespace ndbl;
using namespace fw;

std::string assembly::Instruction::to_string(const Instruction& _instr)
{
    std::string result;
    result.reserve(80); // to fit with terminals

    // append "<line> :"
    std::string str = format::hexadecimal(_instr.line);
    result.append( str );
    result.resize(4, ' ');
    result.append( " : " );

    // append instruction type
    result.append(assembly::to_string(_instr.opcode));

    result.resize(25, ' ');

    // optionally append parameters
    switch ( _instr.opcode )
    {
        case Instruction_t::eval_node:
        {
            result.append(format::hexadecimal(_instr.eval.node.m_value) );
            break;
        }

        case Instruction_t::deref_pool_id:
        {
            result.append(format::hexadecimal( (u64_t)_instr.uref.pool_id ));
            result.append(", *");
            result.append( _instr.uref.type->get_name() );
            break;
        }

        case Instruction_t::mov:
        {
            result.append(assembly::to_string(static_cast<Register>(_instr.mov.dst.u8) ));
            result.append(", ");
            result.append(qword::to_string(_instr.mov.src ));
            break;
        }

        case Instruction_t::cmp:
        {
            result.append(qword::to_string(_instr.cmp.left ));
            result.append(", ");
            result.append(qword::to_string(_instr.cmp.right ));
            break;
        }

        case Instruction_t::jne:
        case Instruction_t::jmp:
        {
            result.append(std::to_string( _instr.jmp.offset ) );
            break;
        }

        case Instruction_t::ret: // nothing else to do.
            break;
        case Instruction_t::pop_stack_frame:
            result.append(format::hexadecimal(_instr.pop.scope.m_value) );
            break;
        case Instruction_t::pop_var:
            result.append(format::hexadecimal(_instr.push.var.m_value) );
            break;
        case Instruction_t::push_stack_frame:
            result.append(format::hexadecimal(_instr.push.scope.m_value) );
            break;
        case Instruction_t::push_var:
            result.append(format::hexadecimal(_instr.push.var.m_value) );
            break;
    }

    // optionally append comment
    if ( !_instr.m_comment.empty() )
    {
        result.resize(50, ' ');

        result.append( "; " );
        result.append( _instr.m_comment );
    }
    result.resize(80, ' '); // to fit with terminals
    return result;
}

