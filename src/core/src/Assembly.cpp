#include <nodable/Assembly.h>

using namespace Nodable;

std::string AssemblyInstr::to_string(const AssemblyInstr& _instr)
{
    std::string result;
    std::string str = std::to_string(_instr.m_line);
    while( str.length() < 4 )
        str.append(" ");
    result.append( str );
    result.append( ": " );

    switch ( _instr.m_type )
    {
        case Instr_call:
        {
            FctId fct_id   = mpark::get<FctId>(_instr.m_left_h_arg );
            Member* member = mpark::get<Member*>(_instr.m_right_h_arg );

            result.append("call ");
            result.append( Nodable::to_string(fct_id) );
            result.append( " [" + std::to_string((size_t)member) + "]");
            break;
        }

        case Instr_mov:
        {
            result.append("mov ");
            result.append(       Nodable::to_string( mpark::get<Register>(_instr.m_left_h_arg ) ) );
            result.append( ", " + Nodable::to_string( mpark::get<Register>(_instr.m_right_h_arg ) ) );
            break;
        }

        case Instr_jne:
        {
            result.append("jne ");
            result.append( std::to_string( mpark::get<long>(_instr.m_left_h_arg ) ) );
            break;
        }

        case Instr_jmp:
        {
            result.append("jmp ");
            result.append( std::to_string( mpark::get<long>(_instr.m_left_h_arg ) ) );
            break;
        }

        case Instr_ret:
        {
            result.append("ret ");
            break;
        }

    }

    if ( !_instr.m_comment.empty() )
    {
        while( result.length() < 50 ) // align on 80th char
            result.append(" ");
        result.append( "; " );
        result.append( _instr.m_comment );
    }
    return result;
}

std::string Nodable::to_string(Register _register)
{
    switch( _register)
    {
        case Register::rax: return "rax";
        case Register::rdx: return "rdx";
        default:            return "???";
    }
}

std::string Nodable::to_string(FctId _id)
{
    switch( _id)
    {
        case FctId::eval_member: return "eval_member";
        default:                 return "???";
    }
}

AssemblyCode::~AssemblyCode()
{
    for( auto each : m_instructions )
        delete each;
    m_instructions.clear();
}

AssemblyInstr* AssemblyCode::push_instr(Instr _type)
{
    AssemblyInstr* instr = new AssemblyInstr(_type, m_instructions.size());
    m_instructions.emplace_back(instr);
    return instr;
}

