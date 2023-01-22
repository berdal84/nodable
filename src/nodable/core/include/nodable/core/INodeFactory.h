#pragma once
#include <string>
#include <memory> // std::shared_ptr
#include <fw/reflection/reflection>

namespace ndbl
{
    // forward declarations
    class Node;
    class IScope;
    class InstructionNode;
    class VariableNode;
    class LiteralNode;
    class ConditionalStructNode;
    class ForLoopNode;

    /**
     * @brief Interface to implement Node factories.
     */
    class INodeFactory
    {
    public:
        virtual Node*                       new_program()const = 0;
        virtual InstructionNode*            new_instr()const = 0;
        virtual VariableNode*				new_variable(fw::type, const std::string&, IScope *)const = 0;
        virtual LiteralNode*                new_literal(fw::type)const = 0;
        virtual Node*                       new_abstract_function(const fw::func_type*, bool _is_operator)const = 0;
        virtual Node*                       new_function(const fw::iinvokable*, bool _is_operator)const = 0;
        virtual Node*                       new_scope()const = 0;
        virtual ConditionalStructNode*      new_cond_struct()const = 0;
        virtual ForLoopNode*                new_for_loop_node()const = 0;
        virtual Node*                       new_node()const = 0;
    };
}

