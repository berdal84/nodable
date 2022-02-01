#pragma once
#include <string>
#include <nodable/Type.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class AbstractScope;
    class InstructionNode;
    class VariableNode;
    class LiteralNode;
    class ConditionalStructNode;
    class ForLoopNode;
    class InvokableOperator;
    class Invokable;
    class Language;

    /**
     * @brief Interface to implement Node factories.
     */
    class AbstractNodeFactory
    {
    public:
        AbstractNodeFactory(const Language* _language): m_language(_language) {};
        virtual ~AbstractNodeFactory() = default;

        virtual Node*                       newProgram()const = 0;
        virtual InstructionNode*		    newInstruction_UserCreated()const = 0;
        virtual InstructionNode*            newInstruction()const = 0;
        virtual VariableNode*				newVariable(Type, const std::string&, AbstractScope *)const = 0;
        virtual LiteralNode*                newLiteral(const Type &type)const = 0;
        virtual Node*                       newBinOp(const InvokableOperator*)const = 0;
        virtual Node*                       newUnaryOp(const InvokableOperator*)const = 0;
        virtual Node*                       newOperator(const InvokableOperator*)const = 0;
        virtual Node*                       newFunction(const Invokable*)const = 0;
        virtual Node*                       newScope()const = 0;
        virtual ConditionalStructNode*      newConditionalStructure()const = 0;
        virtual ForLoopNode*                new_for_loop_node()const = 0;
        virtual Node*                       newNode()const = 0;

    protected:
        // 09/05/2021: I have some doubts, should I keep this pointer here or pass it in method arguments ?
        const Language* m_language;
    };
}

