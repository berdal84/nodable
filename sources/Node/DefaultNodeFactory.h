#include "Node/AbstractNodeFactory.h"

namespace Nodable
{
    /**
     * @brief Node Factory implementation by default.
     */
    class DefaultNodeFactory: public AbstractNodeFactory
    {
    public:
        DefaultNodeFactory(const Language* _language): AbstractNodeFactory(_language) {}
        virtual ~DefaultNodeFactory() {}

        virtual ProgramNode*                newProgram()const;
        virtual CodeBlockNode*              newCodeBlock()const;
        virtual InstructionNode*		    newInstruction_UserCreated()const;
        virtual InstructionNode*            newInstruction()const;
        virtual VariableNode*				newVariable(Type, const std::string&, ScopedCodeBlockNode*)const;
        virtual LiteralNode*                newLiteral(const Type &type)const;
        virtual Node*                       newBinOp(const Operator*)const;
        virtual Node*                       newUnaryOp(const Operator*)const;
        virtual Node*                       newOperator(const Operator*)const;
        virtual Node*                       newFunction(const Function*)const;
        virtual ScopedCodeBlockNode*        newScopedCodeBlock()const;
        virtual ConditionalStructNode*      newConditionalStructure()const;
        virtual Node*                       newNode()const;
    };
}
