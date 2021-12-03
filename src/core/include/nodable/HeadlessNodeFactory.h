#include <nodable/AbstractNodeFactory.h>
#include "Function.h"

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

        ProgramNode*                newProgram()const override ;
        CodeBlockNode*              newCodeBlock()const override ;
        InstructionNode*		    newInstruction_UserCreated()const override ;
        InstructionNode*            newInstruction()const override ;
        VariableNode*				newVariable(Type, const std::string&, ScopedCodeBlockNode*)const override ;
        LiteralNode*                newLiteral(const Type &type)const override ;
        Node*                       newBinOp(const Operator*)const override ;
        Node*                       newUnaryOp(const Operator*)const override ;
        Node*                       newOperator(const Operator*)const override ;
        Node*                       newFunction(const Invokable*)const override ;
        ScopedCodeBlockNode*        newScopedCodeBlock()const override ;
        ConditionalStructNode*      newConditionalStructure()const override ;
        Node*                       newNode()const override ;

    private:
        static void setupNodeLabels(Node *_node, const Operator *_operator);
    };
}
