#include "Code.h"
#include <string>
#include "tools/core/format.h"

using namespace ndbl;
using namespace tools;

Code::~Code()
{
    for( auto each : m_instructions )
    {
        delete each;
    }
    m_instructions.clear();
    m_instructions.resize(0);
}

std::string Code::to_string(const Code* _code)
{
    std::string result;

    result.append( format::title("Program begin") );
    for( Instruction* each_instruction : _code->m_instructions )
    {
        result.append(Instruction::to_string(*each_instruction) );
        result.append("\n");
    }
    result.append( format::title("Program end") );
    return result;
}


