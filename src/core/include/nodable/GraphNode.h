#pragma once

#include <string>
#include <vector>
#include <nodable/Reflect.h>
#include <nodable/IScope.h>
#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <nodable/Node.h>
#include <nodable/Language.h>

namespace Nodable{

    // forward declaration
    class InstructionNode;
    class ConditionalStructNode;
    class LiteralNode;
    class INodeFactory;

    typedef std::pair<const Relation_t, std::pair<Node*, Node*>> Relation;

    inline bool operator==(
            const Relation& _left,
            const Relation& _right)
    {
        return (_left.first == _right.first) && (_left.second == _right.second);
    }

    /**
     * @brief
     * The role of a GraphNode is to manage a set of Node and Wire stored in a m_program_root Node with a given language.
     *
     * @details
     * Nodes and Wires are instantiated and destroyed by this class.
     * The ScopedNode contain the structure of the program in which Nodes are used.
     */
	class GraphNode: public Node {
	public:

		explicit GraphNode(const Language* _language, const INodeFactory* _factory);
		~GraphNode();

		/** Update the graph by evaluating its nodes only when necessary. */
        UpdateResult                update() override;

        /** Clear Graph. Delete all Nodes/Wires and reset scope */
		void                        clear();

        [[nodiscard]] std::vector<Node*>&     getNodeRegistry() {return m_nodeRegistry;}
        [[nodiscard]] std::vector<Wire*>&     getWireRegistry() {return m_wireRegistry;}
        [[nodiscard]] inline const Language*  getLanguage()const { return m_language; }
        [[nodiscard]] inline Node*            getProgram(){ return m_program_root; }
        [[nodiscard]] bool                    hasProgram();
        [[nodiscard]] std::multimap<Relation::first_type , Relation::second_type>& getRelationRegistry() {return m_relationRegistry;}

        /* node factory */
        Node*                       newProgram();
        InstructionNode*		    newInstruction_UserCreated();
        InstructionNode*            newInstruction();
		VariableNode*				newVariable(Reflect::Type, const std::string&, IScope*);
		LiteralNode*                newLiteral(const Reflect::Type &type);
		Node*                       newBinOp(const InvokableOperator*);
		Node*                       newUnaryOp(const InvokableOperator*);
        Node*                       newOperator(const InvokableOperator*);
		Wire*                       newWire();
		Node*                       newFunction(const IInvokable* _proto);
        Node*                       newScope();
        ConditionalStructNode*      newConditionalStructure();
        ForLoopNode*                new_for_loop_node();
        Node*                       newNode();

        /** Connects two Member using a Wire (oriented edge)
         *  If _from is not owned, _to will digest it and nullptr is return.
          * Otherwise a new Wire will be created ( _from -----> _to) and returned.
          */
        Wire* connect(Member *_src_member, Member *_dst_member, ConnBy_ _connect_by = ConnectBy_Ref );

        /**
         * Connect two nodes with a given connection type
         * ex: _source IS_CHILD_OF _target
        */
        void connect(Node* _source, Node* _target, Relation_t, bool _sideEffects = true);
        void connect(Node* _source, InstructionNode* _target);
        void connect(Member* _source, VariableNode* _target);
        void disconnect(Node* _source, Node* _target, Relation_t, bool _sideEffects = true);
        void disconnect(Wire* _wire);
        void disconnect(Member* _member, Way _way = Way_InOut);
        void deleteNode(Node* _node);
    private:
        void registerNode(Node* node);
        void unregisterNode(Node* node);
        void registerWire(Wire *_wire);
        void unregisterWire(Wire *_wire);
        void deleteWire(Wire* _wire);

	private:		
		std::vector<Node*> m_nodeRegistry;
		std::vector<Wire*> m_wireRegistry;
		std::multimap<Relation::first_type , Relation::second_type> m_relationRegistry;
		const Language* m_language;
		Node*           m_program_root;
		const INodeFactory* m_factory;

        REFLECT_DERIVED(GraphNode)
        REFLECT_EXTENDS(Node)
        REFLECT_END
    };
}