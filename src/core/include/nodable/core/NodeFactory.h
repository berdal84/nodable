#pragma once

#include <memory> // std::shared_ptr

#include <nodable/core/INodeFactory.h>
#include <nodable/core/InvokableFunction.h>
#include <nodable/core/IScope.h>
#include <nodable/core/reflection/R.h>

namespace Nodable
{
    /**
     * @brief The NodeFactory instantiate Nodes. Class take a function to apply after creation.
     *
     * By default post processing function does nothing. It can be used to add a NodeView, log messages, etc.
     */
    class NodeFactory: public INodeFactory
    {
    public:
        NodeFactory(const Language* _language, std::function<void(Node *)> _post_process_fct = [](Node* _node){} )
        : m_language(_language)
        , m_post_process(_post_process_fct) {}
        ~NodeFactory() {}

        Node*                       new_program()const override ;
        InstructionNode*            new_instr()const override ;
        VariableNode*				new_variable(std::shared_ptr<const R::MetaType>, const std::string&, IScope *)const override ;
        LiteralNode*                new_literal(std::shared_ptr<const R::MetaType>)const override ;
        Node*                       new_binary_op(const InvokableOperator*)const override;
        Node*                       new_unary_op(const InvokableOperator*)const override;
        Node*                       new_operator(const InvokableOperator*)const override;
        Node*                       new_abstract_function(const FunctionSignature*)const override;
        Node*                       new_function(const IInvokable*)const override ;
        Node*                       new_scope()const override ;
        ConditionalStructNode*      new_cond_struct()const override ;
        ForLoopNode*                new_for_loop_node()const override ;
        Node*                       new_node()const override ;

    private:
        Node*                      _new_abstract_function(const FunctionSignature *_signature) const; // this do not invoke post_process
        static void                setup_node_labels(Node *_node, const InvokableOperator *_operator);

        std::function<void(Node*)> m_post_process; // invoked after each node creation, just before to return.
        const Language*            m_language;

    };
}
