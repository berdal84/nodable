#include <nodable/AbstractNodeFactory.h>
#include <nodable/HeadlessNodeFactory.h>

namespace Nodable
{
    /**
     * @brief Node Factory implementation by default.
     */
    class NodeFactory: public AbstractNodeFactory
    {
    public:
        NodeFactory(const Language* _language)
            : AbstractNodeFactory(_language)
            , m_headless_node_factory(_language) {}

        ~NodeFactory() {}

        ProgramNode*                newProgram()const override ;
        CodeBlockNode*              newCodeBlock()const override ;
        InstructionNode*		    newInstruction_UserCreated()const override ;
        InstructionNode*            newInstruction()const override ;
        VariableNode*				newVariable(Type, const std::string&, ScopedCodeBlockNode*)const override ;
        LiteralNode*                newLiteral(const Type &type)const override ;
        Node*                       newBinOp(const InvokableOperator*)const override ;
        Node*                       newUnaryOp(const InvokableOperator*)const override ;
        Node*                       newOperator(const InvokableOperator*)const override ;
        Node*                       newFunction(const Invokable*)const override ;
        ScopedCodeBlockNode*        newScopedCodeBlock()const override ;
        ConditionalStructNode*      newConditionalStructure()const override ;
        Node*                       newNode()const override ;

    private:
        static void                 AddView(Node* _node);
        HeadlessNodeFactory m_headless_node_factory;
    };
}
