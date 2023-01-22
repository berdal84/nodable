#include <nodable/core/assembly/Code.h>

using namespace ndbl;

assembly::Code::~Code()
{
    for( auto each : m_instructions )
    {
        delete each;
    }
    m_instructions.clear();
    m_instructions.resize(0);
}

std::string assembly::Code::to_string(const Code* _code)
{
    std::string result;

    result.append( "------------<=[ Program begin ]=>------------\n");
    for( Instruction* each_instruction : _code->m_instructions )
    {
        result.append(Instruction::to_string(*each_instruction) );
        result.append("\n");
    }
    result.append("------------<=[ Program end    ]=>------------\n");
    return result;
}


