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
        HeadlessNodeFactory(const Language* _language): m_language(_language) {}
        ~HeadlessNodeFactory() {}

        Node*                       newProgram()const override ;
        InstructionNode*            new_instr()const override ;
        VariableNode*				newVariable(std::shared_ptr<const R::MetaType>, const std::string&, IScope *)const override ;
        LiteralNode*                newLiteral(std::shared_ptr<const R::MetaType>)const override ;
        Node*                       newBinOp(const InvokableOperator*)const override ;
        Node*                       newUnaryOp(const InvokableOperator*)const override ;
        Node*                       newOperator(const InvokableOperator*)const override ;
        Node*                       newFunction(const IInvokable*)const override ;
        Node*                       newScope()const override ;
        ConditionalStructNode*      newConditionalStructure()const override ;
        ForLoopNode*                new_for_loop_node()const override ;
        Node*                       newNode()const override ;

    private:
        static void setupNodeLabels(Node *_node, const InvokableOperator *_operator);
        const Language* m_language;
    };
}
