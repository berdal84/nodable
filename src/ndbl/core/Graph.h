#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include "ndbl/core/Node.h"
#include "ndbl/core/NodeFactory.h"
#include "ndbl/core/WhileLoopNode.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"
#include "tools/core/Optional.h"

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class NodeFactory;
    class GraphView;
    class VariableRefNode;

    typedef int ConnectFlags;
    enum ConnectFlag_
    {
        ConnectFlag_NONE               = 0,
        ConnectFlag_ALLOW_SIDE_EFFECTS = 1 << 0,
    };

    enum CreateNodeType
    {
        CreateNodeType_BLOCK_ENTRY_POINT,
        CreateNodeType_BLOCK_CONDITION,
        CreateNodeType_BLOCK_FOR_LOOP,
        CreateNodeType_BLOCK_WHILE_LOOP,
        CreateNodeType_BLOCK_SCOPE,
        CreateNodeType_VARIABLE_BOOLEAN,
        CreateNodeType_VARIABLE_DOUBLE,
        CreateNodeType_VARIABLE_INTEGER,
        CreateNodeType_VARIABLE_STRING,
        CreateNodeType_LITERAL_BOOLEAN,
        CreateNodeType_LITERAL_DOUBLE,
        CreateNodeType_LITERAL_INTEGER,
        CreateNodeType_LITERAL_STRING,
        CreateNodeType_FUNCTION,
    };

    /**
     * @brief To manage a graph (child_node and edges)
     */
	class Graph
	{
	public:
        typedef std::unordered_set<Node*> NodeRegistry;
        typedef std::multimap<SlotFlags , DirectedEdge> EdgeRegistry;

 		Graph(NodeFactory* factory);
		~Graph();

        // signals (can be connected)

        SIGNAL(on_reset);
        SIGNAL(on_change);
        SIGNAL(on_add   , Node*);
        SIGNAL(on_remove, Node*);

        // general

        bool                     update();
        void                     clear();  // Delete all nodes, wires, edges and reset scope.
        Scope*                   main_scope() { return m_root ? m_root->internal_scope() : nullptr; };
        inline GraphView*        view() const { return m_view; };
        inline void              set_view(GraphView* view = nullptr) { ASSERT(view != nullptr); m_view = view; }
        inline bool              is_empty() const { return m_root.empty(); };
        inline tools::Optional<Node*> root() const { return m_root; }
        inline bool              is_root(const Node* node) const { return m_root == node; }

        // node related

        Node*                    create_node(); // Create a raw node.
        Node*                    create_node(CreateNodeType, const tools::FunctionDescriptor* _signature = nullptr); // Create a given node type in a simple way.
        Node*                    create_entry_point();
        VariableNode*            create_variable(const tools::TypeDescriptor *_type, const std::string &_name);
        VariableRefNode*         create_variable_ref();
        VariableNode*            create_variable_decl(const tools::TypeDescriptor* _type, const char* _name);
        LiteralNode*             create_literal(const tools::TypeDescriptor *_type);
        FunctionNode*            create_function(const tools::FunctionDescriptor*);
        FunctionNode*            create_operator(const tools::FunctionDescriptor*);
        IfNode*                  create_cond_struct();
        ForLoopNode*             create_for_loop();
        WhileLoopNode*           create_while_loop();
        Node*                    create_empty_instruction();
        void                     destroy(Node* _node);
        std::vector<Scope *>     scopes();
        std::set<Scope *>        root_scopes();
        NodeRegistry&            nodes() {return m_node_registry;}
        const NodeRegistry&      nodes()const {return m_node_registry;}
        void                     destroy_next_frame(Node* node) { m_node_to_delete.insert(node ); }

        template<typename T> inline VariableNode* create_variable_decl(const char*  _name = "var"){ return create_variable_decl(tools::type::get<T>(), _name); }
        template<typename T> inline LiteralNode*  create_literal() { return create_literal( tools::type::get<T>()); }

        // edge related

        DirectedEdge  connect(Slot* tail, Slot* head, ConnectFlags = ConnectFlag_NONE );
        void          connect(const std::set<Slot*>& tails, Slot* head, ConnectFlags _flags);
        DirectedEdge  connect_to_variable(Slot* output_slot, VariableNode* variable );
        DirectedEdge  connect_or_merge(Slot* tail, Slot* head);
        void          disconnect( const DirectedEdge& edge, ConnectFlags = ConnectFlag_NONE );
        EdgeRegistry& get_edge_registry() { return m_edge_registry; }

    private:
        void on_disconnect_value_side_effects(DirectedEdge);
        void on_disconnect_flow_side_effects(DirectedEdge);
        void on_connect_value_side_effects(DirectedEdge);
        void on_connect_flow_side_effects(DirectedEdge);

        // registries management
        void add(Node*);           // ... to the registry.
        void remove(Node*);        // ... from the registry.
        DirectedEdge add(const DirectedEdge&);    // ... to the registry.
        void remove(const DirectedEdge&); // ... from the registry.

        tools::Optional<Node*> m_root;
        const NodeFactory* m_factory  = nullptr;
        GraphView*         m_view     = nullptr; // non-owned
        NodeRegistry       m_node_registry;
        NodeRegistry       m_node_to_delete;
        EdgeRegistry       m_edge_registry;
    };
}