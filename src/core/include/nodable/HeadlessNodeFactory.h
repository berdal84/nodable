#include <nodable/AbstractNodeFactory.h>
#include "InvokableFunction.h"
#include "AbstractScope.h"

namespace Nodable
{
    /**
     * @brief Node Factory implementation by default.
     */
    class HeadlessNodeFactory: public AbstractNodeFactory
    {
    public:
        HeadlessNodeFactory(const Language* _language): AbstractNodeFactory(_language) {}
        ~HeadlessNodeFactory() {}

        Node*                       newProgram()const override ;
        InstructionNode*		    newInstruction_UserCreated()const override ;
        InstructionNode*            newInstruction()const override ;
        VariableNode*				newVariable(Type, const std::string&, AbstractScope *)const override ;
        LiteralNode*                newLiteral(const Type &type)const override ;
        Node*                       newBinOp(const InvokableOperator*)const override ;
        Node*                       newUnaryOp(const InvokableOperator*)const override ;
        Node*                       newOperator(const InvokableOperator*)const override ;
        Node*                       newFunction(const Invokable*)const override ;
        Node*                       newScope()const override ;
        ConditionalStructNode*      newConditionalStructure()const override ;
        ForLoopNode*                new_for_loop_node()const override ;
        Node*                       newNode()const override ;

    private:
        static void setupNodeLabels(Node *_node, const InvokableOperator *_operator);
    };
}
