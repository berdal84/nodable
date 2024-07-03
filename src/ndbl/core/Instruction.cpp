#include "Instruction.h"
#include <string>
#include "tools/core/format.h"
#include "Register.h"
#include "ndbl/core/language/Nodlang.h"

using namespace ndbl;
using namespace tools;

std::string Instruction::to_string(const Instruction& _instr)
{
    std::string result;
    result.reserve(80); // to fit with terminals

    // append "<line> :"
    std::string str = format::hexadecimal(_instr.line);
    result.append( str );
    result.resize(4, ' ');
    result.append( " : " );

    // append instruction type
    result.append(ndbl::OpCode_to_string(_instr.opcode));

    result.resize(25, ' ');

    // optionally append parameters
    switch ( _instr.opcode )
    {
        case OpCode_call:
        {
            std::string signature;
            get_language()->serialize_func_sig(signature, _instr.call.func_type );
            result.append(signature);
            break;
        }

        case OpCode_deref_qword:
        {
            result.append(format::address( _instr.uref.ptr ));
            result.append(", *");
            result.append( _instr.uref.type->get_name() );
            break;
        }

        case OpCode_mov:
        {
            result.append(Register_to_string(_instr.mov.dst.u8));
            result.append(", ");
            result.append(qword::to_string(_instr.mov.src ));
            break;
        }

        case OpCode_cmp:
        {
            result.append(qword::to_string(_instr.cmp.left ));
            result.append(", ");
            result.append(qword::to_string(_instr.cmp.right ));
            break;
        }

        case OpCode_jne:
        case OpCode_jmp:
        {
            result.append(std::to_string( _instr.jmp.offset ) );
            break;
        }

        case OpCode_ret: // nothing else to do.
            break;
        case OpCode_pop_stack_frame:
            result.append(format::address(_instr.pop.scope) );
            break;
        case OpCode_pop_var:
            result.append(format::address(_instr.push.var) );
            break;
        case OpCode_push_stack_frame:
            result.append(format::address(_instr.push.scope) );
            break;
        case OpCode_push_var:
            result.append(format::address(_instr.push.var) );
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

