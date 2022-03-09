#pragma once
#include <string>
#include <memory> // std::shared_ptr
#include <nodable/R.h>

namespace Nodable
{
    // forward declarations
    class Node;
    class IScope;
    class InstructionNode;
    class VariableNode;
    class LiteralNode;
    class ConditionalStructNode;
    class ForLoopNode;
    class InvokableOperator;
    class IInvokable;
    class Language;

    /**
     * @brief Interface to implement Node factories.
     */
    class INodeFactory
    {
    public:
        virtual Node*                       newProgram()const = 0;
        virtual InstructionNode*            new_instr()const = 0;
        virtual VariableNode*				newVariable(std::shared_ptr<const R::MetaType>, const std::string&, IScope *)const = 0;
        virtual LiteralNode*                newLiteral(std::shared_ptr<const R::MetaType>)const = 0;
        virtual Node*                       newBinOp(const InvokableOperator*)const = 0;
        virtual Node*                       newUnaryOp(const InvokableOperator*)const = 0;
        virtual Node*                       newOperator(const InvokableOperator*)const = 0;
        virtual Node*                       newFunction(const IInvokable*)const = 0;
        virtual Node*                       newScope()const = 0;
        virtual ConditionalStructNode*      newConditionalStructure()const = 0;
        virtual ForLoopNode*                new_for_loop_node()const = 0;
        virtual Node*                       newNode()const = 0;
    };
}

