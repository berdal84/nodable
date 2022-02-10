#include <nodable/INodeFactory.h>
#include <nodable/HeadlessNodeFactory.h>
#include <nodable/Reflect.h>

namespace Nodable
{
    /**
     * @brief Node Factory implementation by default.
     */
    class NodeFactory: public INodeFactory
    {
    public:
        NodeFactory(const Language* _language)
            : INodeFactory(_language)
            , m_headless_node_factory(_language) {}

        ~NodeFactory() {}

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
        static void                 AddView(Node* _node);
        HeadlessNodeFactory m_headless_node_factory;
    };
}
