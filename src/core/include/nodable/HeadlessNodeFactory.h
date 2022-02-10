#include <nodable/INodeFactory.h>

#include <nodable/InvokableFunction.h>
#include <nodable/IScope.h>
#include <nodable/Reflect.h>

namespace Nodable
{
    /**
     * @brief Node Factory implementation by default.
     */
    class HeadlessNodeFactory: public INodeFactory
    {
    public:
        HeadlessNodeFactory(const Language* _language): INodeFactory(_language) {}
        ~HeadlessNodeFactory() {}

        Node*                       newProgram()const override ;
        InstructionNode*		    newInstruction_UserCreated()const override ;
        InstructionNode*            newInstruction()const override ;
        VariableNode*				newVariable(Reflect::Type, const std::string&, IScope *)const override ;
        LiteralNode*                newLiteral(const Reflect::Type &type)const override ;
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
    };
}
