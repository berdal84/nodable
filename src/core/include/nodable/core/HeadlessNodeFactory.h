#pragma once

#include <memory> // std::shared_ptr

#include <nodable/core/INodeFactory.h>
#include <nodable/core/InvokableFunction.h>
#include <nodable/core/IScope.h>
#include <nodable/core/reflection/R.h>

namespace Nodable
{
    /**
     * @brief Node Factory implementation by default.
     */
    class HeadlessNodeFactory: public INodeFactory
    {
    public:
        HeadlessNodeFactory(const Language* _language, std::function<void(Node*)> _post_process_fct = [](Node* _node){} )
        : m_language(_language)
        , m_post_process(_post_process_fct) {}
        ~HeadlessNodeFactory() {}

        Node*                       new_program()const override ;
        InstructionNode*            new_instr()const override ;
        VariableNode*				new_variable(std::shared_ptr<const R::MetaType>, const std::string&, IScope *)const override ;
        LiteralNode*                new_literal(std::shared_ptr<const R::MetaType>)const override ;
        Node*                       new_binary_op(const InvokableOperator*)const override;
        Node*                       new_unary_op(const InvokableOperator*)const override;
        Node*                       new_operator(const InvokableOperator*)const override;
        Node*                       new_function(const FunctionSignature*)const override;
        Node*                       new_function(const IInvokable*)const override ;
        Node*                       new_scope()const override ;
        ConditionalStructNode*      new_cond_struct()const override ;
        ForLoopNode*                new_for_loop_node()const override ;
        Node*                       new_node()const override ;

    private:
        static void                setup_node_labels(Node *_node, const InvokableOperator *_operator);

        std::function<void(Node*)> m_post_process;
        const Language*            m_language;

    };
}
