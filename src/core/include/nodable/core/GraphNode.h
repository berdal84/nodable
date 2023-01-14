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

namespace ndbl
{
    /**
     * @brief a GraphNode is a context for a set of Nodes and Wires. It is also used to drop_on Nodes and Members.
     */
	class GraphNode: public Node
	{
	public:
        using RelationRegistry_t = std::multimap<EdgeType, const DirectedEdge>; // relation storage (by edge type)

		explicit GraphNode(const ILanguage*, const INodeFactory*, const bool* _autocompletion);
		~GraphNode();

        UpdateResult                update() override;                                          // Delete/Update nodes if necessary.

        // node related

        Node*                       create_root();                                              // Create a new root instruction in this graph.
        InstructionNode*            create_instr();                                             // Create a new instruction in this graph.
		VariableNode*				create_variable(type, const std::string&, IScope*);         // Create a new variable of a given type, identifier and scope (use create_scope()) in this graph.
		LiteralNode*                create_literal(type);                                       // Create a new literal of a given type.
		Node*                       create_abstract_function(const func_type*, bool _is_operator = false); // Create and append a new abstract (without known implementation)  function of a given type.
		Node*                       create_function(const iinvokable*, bool _is_operator = false); // Create a new (invokable) function.
        Node*                       create_abstract_operator(const func_type*);                 // Create a new abstract (without known implementation) operator.
        Node*                       create_operator(const iinvokable*);                         // Create a new operator.
        Node*                       create_scope();                                             // Create a new scope.
        ConditionalStructNode*      create_cond_struct();                                       // Create a new conditional structure.
        ForLoopNode*                create_for_loop();                                          // Create a new for loop (iterative structure).
        Node*                       create_node();                                              // Create a new node (simplest possible).
        void                        destroy(Node*);                                             // Delete a given node after disconnecting all its edges/wires.
        void                        ensure_has_root();                                          // If necessary, create a root node.
        Node*                       get_root()const { return m_root; }                          // Get root node.
        bool                        is_empty() const;                                           // Check if graph has a root (orphan nodes are ignored).

        // wire or edges related

        void                        connect(DirectedEdge, bool _side_effects = true);           // Create a new directed edge, side effects can be turned off.
        void                        disconnect(DirectedEdge, bool _side_effects = true);        // Disconnect a directed edge, side effects can be turned off.
        Wire*                       connect(Member* _src, Member* _dst_member );                // Create a wire between a source and a destination member (source and destination must be different).
        void                        disconnect(Wire*);                                          // Disconnect a given wire.
        void                        connect(Node* _src, InstructionNode* _dst);                 // Connect a given node to an instruction node. The node will be the root expression of that instruction.
        void                        connect(Member* _src, VariableNode* _dst);                  // Connect a node's member to a given variable. This member will be the initial value of the variable.
        void                        destroy(Wire*);
        std::vector<Wire*>          filter_wires(Member*, Way) const;                           // Filter all the wires connected to a given member in certain way.

    private:

        // registers management

        void                        add(Node*);                                                  // Add a given node to the node registry.
        void                        remove(Node*);                                               // Remove a given node from the node registry.
        void                        add(Wire*);                                                  // Add a given wire to the wire registry.
        void                        remove(Wire*);                                               // Remove a given wire from the wire registry.
    public:
        void                        clear();                                                     // Delete all nodes, wires, relations and reset scope.
        const std::vector<Node*>&   get_node_registry()const {return m_node_registry;}           // Get the node registry.
        const std::vector<Wire*>&   get_wire_registry()const {return m_wire_registry;}           // Get the wire registry.
        RelationRegistry_t&         get_relation_registry() {return m_relation_registry;}        // Get the relation registry.

	private:		
		std::vector<Node*>       m_node_registry;       // registry to store all the nodes from this graph.
		std::vector<Wire*>       m_wire_registry;       // registry to store all wires connected to the registered nodes. TODO: Can't we merge this with m_relation_registry?
		RelationRegistry_t       m_relation_registry;   // registry ot all the relations (directed edges) between the registered nodes.
		const ILanguage*         m_language;
		Node*                    m_root;                // Graph root (main scope), without it a graph cannot be compiled.
		const INodeFactory*      m_factory;             // Node factory (can be headless or not depending on the context: app, unit tests, cli).
        const bool*              m_autocompletion;      // Abandoned idea about autocompleting graph.

        REFLECT_DERIVED_CLASS(Node)

    };
}