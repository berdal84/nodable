#pragma once

#include <nodable/INodeFactory.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/R.h>
#include <nodable/AppContext.h>

namespace Nodable
{

    /**
     * @brief Node Factory implementation by default.
     */
    class AppNodeFactory: public INodeFactory
    {
    public:
        AppNodeFactory(AppContext* _ctx)
            : m_context(_ctx)
            , m_headless_node_factory(_ctx->language) {}

        ~AppNodeFactory() {}

        Node*                       newProgram()const override ;
        InstructionNode*            new_instr()const override ;
        VariableNode*				newVariable(const R::Type*, const std::string&, IScope *)const override ;
        LiteralNode*                newLiteral(const R::Type* type)const override ;
        Node*                       newBinOp(const InvokableOperator*)const override ;
        Node*                       newUnaryOp(const InvokableOperator*)const override ;
        Node*                       newOperator(const InvokableOperator*)const override ;
        Node*                       newFunction(const IInvokable*)const override ;
        Node*                       newScope()const override ;
        ConditionalStructNode*      newConditionalStructure()const override ;
        ForLoopNode*                new_for_loop_node()const override ;
        Node*                       newNode()const override ;

    private:
        void                        post_instantiation(Node* _node)const;
        HeadlessNodeFactory m_headless_node_factory;
        AppContext*         m_context;
    };
}
