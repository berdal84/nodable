#pragma once

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include <nodable/core/reflection/reflection>
#include <nodable/core/IScope.h>
#include <nodable/core/types.h>
#include <nodable/core/Component.h>
#include <nodable/core/Node.h>
#include <nodable/core/ILanguage.h>
#include <nodable/core/INodeFactory.h>

namespace Nodable
{
    /**
     * @brief a GraphNode is a context for a set of Nodes and Wires. It is also used to drop_on Nodes and Members.
     */
	class GraphNode: public Node
	{
	public:
        using RelationRegistry_t = std::multimap<EdgeType, const DirectedEdge>;

		explicit GraphNode(const ILanguage*, const INodeFactory*, const bool* _autocompletion);
		~GraphNode();

        UpdateResult                update() override; // Update the graph by evaluating its nodes only when necessary.
		void                        clear(); // Clear Graph. Delete all Nodes/Wires and reset scope
        const std::vector<Node*>&   get_node_registry()const {return m_node_registry;}
        const std::vector<Wire*>&   get_wire_registry()const {return m_wire_registry;}
        Node*                       get_root()const { return m_root; }
        RelationRegistry_t&         get_relation_registry() {return m_relation_registry;}
        bool                        is_empty() const;
        void                        ensure_has_root();
        Node*                       create_root();
        InstructionNode*            create_instr();
		VariableNode*				create_variable(type, const std::string&, IScope*);

		template<typename T>
		VariableNode*				create_variable(const std::string& _name, IScope* _scope)
        {
		    return create_variable(type::get<T>(), _name, _scope);
        }

		LiteralNode*                create_literal(type);
 		Wire*                       create_wire();
		Node*                       create_abstract_function(const Signature*);
		Node*                       create_function(const IInvokable*);
        Node*                       create_scope();
        ConditionalStructNode*      create_cond_struct();
        ForLoopNode*                create_for_loop();
        Node*                       create_node();

        void connect(DirectedEdge, bool _side_effects = true);
        void disconnect(DirectedEdge, bool _side_effects = true);
        Wire* connect(Member* _src, Member* _dst_member );
        void disconnect(Wire*);
        void disconnect(Member* _member, Way _way = Way_InOut, bool _side_effects = true);
        void connect(Node* _src, InstructionNode* _dst);
        void connect(Member* _src, VariableNode* _dst);

        void destroy(Node*);
        std::vector<Wire*> filter_wires(Member*, Way) const;
    private:
        void add(Node*);
        void remove(Node*);
        void add(Wire*);
        void remove(Wire*);
        void destroy(Wire*);

	private:		
		std::vector<Node*> m_node_registry;
		std::vector<Wire*> m_wire_registry;
		RelationRegistry_t m_relation_registry;
		const ILanguage*    m_language;
		Node*              m_root;
		const INodeFactory* m_factory;
        const bool* m_autocompletion;

        REFLECT_DERIVED_CLASS(Node)

    };
}