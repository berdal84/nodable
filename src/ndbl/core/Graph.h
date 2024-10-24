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

#include "IScope.h"
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
        CreateNodeType_BLOCK_CONDITION,
        CreateNodeType_BLOCK_FOR_LOOP,
        CreateNodeType_BLOCK_WHILE_LOOP,
        CreateNodeType_BLOCK_SCOPE,
        CreateNodeType_BLOCK_PROGRAM,
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
     * @brief To manage a graph (nodes and edges)
     */
	class Graph
	{
	public:
 		Graph(NodeFactory* factory);
		~Graph();

        // signals (can be connected)

        SIGNAL(on_reset);
        SIGNAL(on_update);
        SIGNAL(on_add   , Node*);
        SIGNAL(on_remove, Node*);

        // general

        bool                     update();
        void                     clear();  // Delete all nodes, wires, edges and reset scope.
        inline GraphView*        view() const { return m_view; };
        inline void              set_view(GraphView* view = nullptr) { ASSERT(view != nullptr); m_view = view; }
        inline bool              is_empty() const { return m_root.empty(); };
        inline bool              is_dirty() const { return m_is_dirty; }
        inline void              set_dirty(bool b = true) { m_is_dirty = b; }
        inline void              ensure_has_root() { if (is_empty()) create_root(); }
        inline tools::Optional<Node*> root() const { return m_root; }
        inline bool              is_root(const Node* node) const { return  m_root == node; }

        // node related

        Node*                    create_node(); // Create a raw node.
        Node*                    create_node(CreateNodeType, const tools::FunctionDescriptor* _signature = nullptr); // Create a given node type in a simple way.
        Node*                    create_root();
        VariableNode*            create_variable(const tools::TypeDescriptor *_type, const std::string &_name, Scope* _scope);
        VariableRefNode*         create_variable_ref();
        VariableNode*            create_variable_decl(const tools::TypeDescriptor* _type, const char*  _name, Scope*  _scope);
        LiteralNode*             create_literal(const tools::TypeDescriptor *_type);
        FunctionNode*            create_function(const tools::FunctionDescriptor*);
        FunctionNode*            create_operator(const tools::FunctionDescriptor*);
        Node*                    create_scope();
        IfNode*                  create_cond_struct();
        ForLoopNode*             create_for_loop();
        WhileLoopNode*           create_while_loop();
        void                     destroy(Node* _node);
        inline std::vector<Node*>&       get_node_registry() {return m_node_registry;}
        inline const std::vector<Node*>& get_node_registry()const {return m_node_registry;}

        template<typename T> inline VariableNode* create_variable_decl(const char*  _name = "var", Scope* _scope = {}){ return create_variable_decl(tools::type::get<T>(), _name, _scope); }
        template<typename T> inline LiteralNode* create_literal() { return create_literal( tools::type::get<T>()); }

        // edge related

        DirectedEdge connect(Slot* tail, Slot* head, ConnectFlags = ConnectFlag_NONE );
        DirectedEdge connect_to_variable(Slot* output_slot, VariableNode* variable );
        DirectedEdge connect_or_merge(Slot* tail, Slot* head);
        void         disconnect( const DirectedEdge& edge, ConnectFlags = ConnectFlag_NONE );
        inline std::multimap<SlotFlags, DirectedEdge>& get_edge_registry() { return m_edge_registry; }

    private:
        void on_connect_hierarchical_side_effects(Slot* parent_slot, Slot* child_slot);
        void on_connect_value_side_effects(Slot* out_slot, Slot* in_slot);
        void on_connect_codeflow_side_effects(Slot* prev_slot, Slot* next_slot);

        // registries management
        void add(Node*);           // ... to the registry.
        void remove(Node*);        // ... from the registry.
        void add(const DirectedEdge&);    // ... to the registry.
        void remove(const DirectedEdge&); // ... from the registry.

        tools::Optional<Node*> m_root = nullptr; // Graph root (main scope), without it a graph cannot be compiled.
        const NodeFactory* m_factory  = nullptr;
        bool               m_is_dirty = false;
        GraphView*         m_view     = nullptr; // non-owned
        std::vector<Node*>                      m_node_registry; // Node storage
        std::multimap<SlotFlags , DirectedEdge> m_edge_registry; // Edge storage
    };
}