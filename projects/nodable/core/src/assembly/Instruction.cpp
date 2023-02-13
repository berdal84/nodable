#include <ndbl/core/assembly/Register.h>
#include <ndbl/core/assembly/Instruction.h>
#include "fw/core/String.h"

using namespace ndbl;
using namespace fw;

std::string assembly::Instruction::to_string(const Instruction& _instr)
{
    std::string result;
    result.reserve(80); // to fit with terminals

    // append "<line> :"
    std::string str = String::fmt_hex(_instr.line);
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
            result.append(String::fmt_ptr(_instr.eval.node) );
            break;
        }

        case Instruction_t::deref_ptr:
        {
            FW_EXPECT(_instr.uref.qword_ptr, "qword_ptr is null!")
            result.append(qword::to_string(*_instr.uref.qword_ptr ));
            result.append(", *");
            result.append( _instr.uref.qword_type->get_name() );
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
            result.append(String::fmt_ptr(_instr.pop.scope) );
            break;
        case Instruction_t::pop_var:
            result.append(String::fmt_ptr(_instr.push.var) );
            break;
        case Instruction_t::push_stack_frame:
            result.append(String::fmt_ptr(_instr.push.scope) );
            break;
        case Instruction_t::push_var:
            result.append(String::fmt_ptr(_instr.push.var) );
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

