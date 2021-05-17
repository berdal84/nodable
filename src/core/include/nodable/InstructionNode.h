#pragma once

#include <string>

#include <nodable/Nodable.h> // forward declarations and common stuff
#include <nodable/Node.h> // base class
#include <nodable/Member.h>

namespace Nodable::core
{
    // forward declarations
    struct Token;
    class CodeBlockNode;

    /*
        The role of this class is to symbolize an instruction.
        The result of the instruction is value()
    */
    class InstructionNode : public Node
    {
    public:
        explicit InstructionNode(const char* _label);
        ~InstructionNode()= default;

        [[nodiscard]] inline Member* getValue()const { return m_props.get("value"); }
                      inline void    setValue(Member* _value) const { getValue()->set(_value); };
        [[nodiscard]] inline Token*  getEndOfInstrToken()const { return m_endOfInstrToken; }
                      inline void    setEndOfInstrToken(Token* token) { m_endOfInstrToken = token; }

    private:
        Token* m_endOfInstrToken = nullptr;

        /** reflect class using mirror */
        MIRROR_CLASS(InstructionNode)
        (
            MIRROR_PARENT(Node)
        );
    };
}
