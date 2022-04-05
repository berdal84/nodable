#pragma once
#include <string>
#include <memory> // std::shared_ptr
#include <nodable/core/reflection/R.h>

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
    class IInvokable;
    class Language;
    class Signature;
    class Operator;

    /**
     * @brief Interface to implement Node factories.
     */
    class INodeFactory
    {
    public:
        virtual Node*                       new_program()const = 0;
        virtual InstructionNode*            new_instr()const = 0;
        virtual VariableNode*				new_variable(std::shared_ptr<const R::MetaType>, const std::string&, IScope *)const = 0;
        virtual LiteralNode*                new_literal(std::shared_ptr<const R::MetaType>)const = 0;
        virtual Node*                       new_abstract_function(const Signature*)const = 0;
        virtual Node*                       new_function(const IInvokable*)const = 0;
        virtual Node*                       new_scope()const = 0;
        virtual ConditionalStructNode*      new_cond_struct()const = 0;
        virtual ForLoopNode*                new_for_loop_node()const = 0;
        virtual Node*                       new_node()const = 0;
    };
}

