#pragma once

#include <string>
#include <vector>
#include <mirror.h>
#include <nodable/AbstractCodeBlockNode.h>

#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <nodable/Node.h>
#include <nodable/Language.h>

namespace Nodable{

    // forward declaration
    class ScopedCodeBlockNode;
    class InstructionNode;
    class CodeBlockNode;
    class ConditionalStructNode;
    class ProgramNode;
    class LiteralNode;
    class AbstractNodeFactory;

    typedef std::pair<const RelationType, std::pair<Node*, Node*>> Relation;

    inline bool operator==(
            const Relation& _left,
            const Relation& _right)
    {
        return (_left.first == _right.first) && (_left.second == _right.second);
    }

    /**
     * @brief
     * The role of a GraphNode is to manage a set of Node and Wire used in a ScopedCodeBlockNode with a given language.
     *
     * @details
     * Nodes and Wires are instantiated and destroyed by this class.
     * The ScopedCodeBlockNode contain the structure of the program in which Nodes are used.
     */
	class GraphNode: public Node {
	public:

		explicit GraphNode(const Language* _language, const AbstractNodeFactory* _factory);
		~GraphNode();

		/** Update the graph by evaluating its nodes only when necessary. */
        UpdateResult                update() override;

        /** Clear Graph. Delete all Nodes/Wires and reset scope */
		void                        clear();

		/** Find a VariableNode given its name/identifier
		 * @return - a valid VariableNode* or nullptr if variable is not found. */
        [[nodiscard]] VariableNode*           findVariable(std::string);

        [[nodiscard]] std::vector<Node*>&     getNodeRegistry() {return m_nodeRegistry;}
        [[nodiscard]] std::vector<Wire*>&     getWireRegistry() {return m_wireRegistry;}
        [[nodiscard]] inline const Language*  getLanguage()const { return m_language; }
        [[nodiscard]] inline ProgramNode*     getProgram(){ return m_program; }
        [[nodiscard]] bool                    hasProgram();
        [[nodiscard]] std::multimap<Relation::first_type , Relation::second_type>& getRelationRegistry() {return m_relationRegistry;}

        /* node factory */
        ScopedCodeBlockNode*        newProgram();
		CodeBlockNode*              newCodeBlock();
        InstructionNode*		    newInstruction_UserCreated();
        InstructionNode*            newInstruction();
		VariableNode*				newVariable(Type, const std::string&, ScopedCodeBlockNode*);
		LiteralNode*                newLiteral(const Type &type);
		Node*                       newBinOp(const Operator*);
		Node*                       newUnaryOp(const Operator*);
        Node*                       newOperator(const Operator*);
		Wire*                       newWire();
		Node*                       newFunction(const Function* _proto);
        ScopedCodeBlockNode*        newScopedCodeBlock();
        ConditionalStructNode*      newConditionalStructure();
        Node*                       newNode();

        /** Connects two Member using a Wire (oriented edge)
         *  If _from is not owned, _to will digest it and nullptr is return.
          * Otherwise a new Wire will be created ( _from -----> _to) and returned.
          */
        Wire* connect(Member *_source, Member *_target);

        /**
         * Connect two nodes with a given connection type
         * ex: _source IS_CHILD_OF _target
        */
        void connect(Node* _source, Node* _target, RelationType, bool _sideEffects = true);
        void connect(Member* _source, InstructionNode* _target);
        void disconnect(Node* _source, Node* _target, RelationType, bool _sideEffects = true);
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
		const Language*            m_language;
		ProgramNode*               m_program;
		const AbstractNodeFactory* m_factory;
	public:
    /** reflect class with mirror */
    MIRROR_CLASS(GraphNode)
    (
        MIRROR_PARENT(Node) // we only need to know parent
    )

    };
}