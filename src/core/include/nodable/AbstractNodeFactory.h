#pragma once
#include <string>
#include <nodable/Type.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class ScopedCodeBlockNode;
    class InstructionNode;
    class VariableNode;
    class LiteralNode;
    class ConditionalStructNode;
    class CodeBlockNode;
    class Operator;
    class Invokable;
    class ProgramNode;
    class Language;

    /**
     * @brief Interface to implement Node factories.
     */
    class AbstractNodeFactory
    {
    public:
        AbstractNodeFactory(const Language* _language): m_language(_language) {};
        virtual ~AbstractNodeFactory() = default;

        virtual ProgramNode*                newProgram()const = 0;
        virtual CodeBlockNode*              newCodeBlock()const = 0;
        virtual InstructionNode*		    newInstruction_UserCreated()const = 0;
        virtual InstructionNode*            newInstruction()const = 0;
        virtual VariableNode*				newVariable(Type, const std::string&, ScopedCodeBlockNode*)const = 0;
        virtual LiteralNode*                newLiteral(const Type &type)const = 0;
        virtual Node*                       newBinOp(const Operator*)const = 0;
        virtual Node*                       newUnaryOp(const Operator*)const = 0;
        virtual Node*                       newOperator(const Operator*)const = 0;
        virtual Node*                       newFunction(const Invokable*)const = 0;
        virtual ScopedCodeBlockNode*        newScopedCodeBlock()const = 0;
        virtual ConditionalStructNode*      newConditionalStructure()const = 0;
        virtual Node*                       newNode()const = 0;

    protected:
        // 09/05/2021: I have some doubts, should I keep this pointer here or pass it in method arguments ?
        const Language* m_language;
    };
}

