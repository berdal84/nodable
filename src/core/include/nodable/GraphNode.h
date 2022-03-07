#pragma once

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include <nodable/R.h>
#include <nodable/IScope.h>
#include <nodable/Nodable.h>
#include <nodable/Component.h>
#include <nodable/Node.h>
#include <nodable/Language.h>
#include <nodable/INodeFactory.h>

namespace Nodable
{
    /**
     * @brief a GraphNode is a context for a set of Nodes and Wires. It is also used to connect Nodes and Members.
     */
	class GraphNode: public Node
	{
	public:

		explicit GraphNode(const Language*, const INodeFactory*, const bool* _autocompletion);
		~GraphNode();

		/** Update the graph by evaluating its nodes only when necessary. */
        UpdateResult            update() override;

        /** Clear Graph. Delete all Nodes/Wires and reset scope */
		void                    clear();

        std::vector<Node*>&     get_node_registry() {return m_node_registry;}
        std::vector<Wire*>&     get_wire_registry() {return m_wire_registry;}
        const Language*         get_language()const { return m_language; }
        Node*                   get_root() { return m_root; }
        RelationRegistry&       get_relation_registry() {return m_relation_registry;}
        bool                    is_empty();
        void                    ensure_has_root();
        /* node factory */
        Node*                       create_root();
        InstructionNode*            create_instr();
		VariableNode*				create_variable(std::shared_ptr<const R::Type>, const std::string&, IScope*);
		LiteralNode*                create_literal(std::shared_ptr<const R::Type>);
		Node*                       create_bin_op(const InvokableOperator*);
		Node*                       create_unary_op(const InvokableOperator*);
        Node*                       create_operator(const InvokableOperator*);
		Wire*                       create_wire();
		Node*                       create_function(const IInvokable*);
        Node*                       create_scope();
        ConditionalStructNode*      create_cond_struct();
        ForLoopNode*                create_for_loop();
        Node*                       create_node();

        /** Connects two Member using a Wire (oriented edge)
         *  If _from is not owned, _to will digest it and nullptr is return.
          * Otherwise a new Wire will be created ( _from -----> _to) and returned.
          */
        Wire* connect(Member* _src, Member* _dst, ConnBy_ _connect_by = ConnectBy_Ref );

        /**
         * connect two nodes with a given connection type
         * ex: _source IS_CHILD_OF _target
        */
        void connect(Node* _src, Node* _dst, Relation_t, bool _side_effects = true);
        void connect(Node* _src, InstructionNode* _dst);
        void connect(Member* _src, VariableNode* _dst);
        void disconnect(Node* _src, Node* _dst, Relation_t, bool _side_effects = true);
        void disconnect(Wire*);
        void disconnect(Member* _member, Way _way = Way_InOut);
        void destroy(Node*);
    private:
        void add(Node*);
        void remove(Node*);
        void add(Wire*);
        void remove(Wire*);
        void destroy(Wire*);

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