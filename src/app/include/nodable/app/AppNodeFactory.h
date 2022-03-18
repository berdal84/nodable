#pragma once

#include <memory>
#include <nodable/core/INodeFactory.h>
#include <nodable/core/HeadlessNodeFactory.h>
#include <nodable/core/reflection/R.h>
#include <nodable/app/AppContext.h>

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
        VariableNode*				newVariable(std::shared_ptr<const R::MetaType>, const std::string&, IScope *)const override ;
        LiteralNode*                newLiteral(std::shared_ptr<const R::MetaType> type)const override ;
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
