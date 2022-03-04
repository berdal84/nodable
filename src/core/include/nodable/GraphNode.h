#pragma once

#include <string>
#include <vector>
#include <nodable/R.h>
#include <nodable/IScope.h>
#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <nodable/Node.h>
#include <nodable/Language.h>
#include <nodable/INodeFactory.h>
#include <nodable/IGraph.h>

namespace Nodable
{
    /**
     * @brief a GraphNode is a context for a set of Nodes and Wires. It is also used to connect Nodes and Members.
     */
	class GraphNode: public Node, public IGraph
	{
	public:

		explicit GraphNode(const Language*, const INodeFactory*, const bool* _autocompletion);
		~GraphNode();

		/** Update the graph by evaluating its nodes only when necessary. */
        UpdateResult            update() override;

        /** Clear Graph. Delete all Nodes/Wires and reset scope */
		void                    clear() override;

        std::vector<Node*>&     get_node_registry() override {return m_node_registry;}
        std::vector<Wire*>&     get_wire_registry() override {return m_wire_registry;}
        const Language*         get_language()const override { return m_language; }
        Node*                   get_root() override { return m_root; }
        RelationRegistry&       get_relation_registry() override {return m_relation_registry;}
        bool                    is_empty() override;
        void                    ensure_has_root() override;
        /* node factory */
        Node*                       create_root() override;
        InstructionNode*            create_instr() override;
		VariableNode*				create_variable(const R::Type*, const std::string&, IScope*) override;
		LiteralNode*                create_literal(const R::Type*) override;
		Node*                       create_bin_op(const InvokableOperator*) override;
		Node*                       create_unary_op(const InvokableOperator*) override;
        Node*                       create_operator(const InvokableOperator*) override;
		Wire*                       create_wire() override;
		Node*                       create_function(const IInvokable*) override;
        Node*                       create_scope() override;
        ConditionalStructNode*      create_cond_struct() override;
        ForLoopNode*                create_for_loop() override;
        Node*                       create_node() override;

        /** Connects two Member using a Wire (oriented edge)
         *  If _from is not owned, _to will digest it and nullptr is return.
          * Otherwise a new Wire will be created ( _from -----> _to) and returned.
          */
        Wire* connect(Member* _src, Member* _dst, ConnBy_ _connect_by = ConnectBy_Ref ) override;

        /**
         * Connect two nodes with a given connection type
         * ex: _source IS_CHILD_OF _target
        */
        void connect(Node* _src, Node* _dst, Relation_t, bool _side_effects = true) override;
        void connect(Node* _src, InstructionNode* _dst) override;
        void connect(Member* _src, VariableNode* _dst) override;
        void disconnect(Node* _src, Node* _dst, Relation_t, bool _side_effects = true) override;
        void disconnect(Wire*) override;
        void disconnect(Member* _member, Way _way = Way_InOut) override;
        void destroy(Node*) override;
    private:
        void add(Node*) override;
        void remove(Node*) override;
        void add(Wire*) override;
        void remove(Wire*) override;
        void destroy(Wire*) override;

	private:		
		std::vector<Node*> m_node_registry;
		std::vector<Wire*> m_wire_registry;
		RelationRegistry   m_relation_registry;
		const Language*    m_language;
		Node*              m_root;
		const INodeFactory* m_factory;
        const bool* m_autocompletion;

        R_DERIVED(GraphNode)
        R_EXTENDS(Node)
        R_END
    };
}