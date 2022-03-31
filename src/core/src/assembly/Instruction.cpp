#include "nodable/core/assembly/Instruction.h"
#include "nodable/core/Format.h"

using namespace Nodable;

std::string assembly::Instruction::to_string(const Instruction& _instr)
{
    std::string result;
    result.reserve(80); // to fit with terminals

    // append "<line> :"
    std::string str = Format::fmt_hex(_instr.line);
    result.append( str );
    result.resize(4, ' ');
    result.append( " : " );

    // append instruction type
    result.append(assembly::to_string(_instr.type));

    result.resize(25, ' ');

    // optionally append parameters
    switch ( _instr.type )
    {
        case opcode::eval_node:
        {
            result.append(Format::fmt_ptr(_instr.eval.node) );
            break;
        }

        case opcode::deref_ptr:
        {
            result.append(assembly::QWord::to_string(*_instr.uref.qword_ptr ));
            result.append(", *");
            result.append( R::to_string(_instr.uref.qword_type));
            break;
        }

        case opcode::mov:
        {
            result.append(assembly::to_string(_instr.mov.dst.r ));
            result.append(", ");
            result.append(assembly::QWord::to_string(_instr.mov.src ));
            break;
        }

        case opcode::cmp:
        {
            result.append(assembly::QWord::to_string(_instr.cmp.left ));
            result.append(", ");
            result.append(assembly::QWord::to_string(_instr.cmp.right ));
            break;
        }

        case opcode::jne:
        case opcode::jmp:
        {
            result.append( std::to_string( _instr.jmp.offset ) );
            break;
        }

        case opcode::ret: // nothing else to do.
            break;
        case opcode::pop_stack_frame:
            result.append(Format::fmt_ptr(_instr.pop.scope) );
            break;
        case opcode::pop_var:
            result.append(Format::fmt_ptr(_instr.push.var) );
            break;
        case opcode::push_stack_frame:
            result.append(Format::fmt_ptr(_instr.push.scope) );
            break;
        case opcode::push_var:
            result.append(Format::fmt_ptr(_instr.push.var) );
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

