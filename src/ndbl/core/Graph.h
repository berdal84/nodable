#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory> // std::shared_ptr

#include "ndbl/core/ASTNode.h"
#include "ndbl/core/ASTNodeFactory.h"
#include "ndbl/core/ASTWhileLoop.h"
#include "tools/core/reflection/reflection"
#include "tools/core/types.h"
#include "tools/core/Optional.h"

namespace ndbl
{
    // forward declarations
    class Nodlang;
    class ASTNodeFactory;
    class GraphView;
    class ASTVariableRef;

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
     * @brief To manage a graph (child and edges)
     */
	class Graph
	{
	public:
        typedef std::unordered_set<ASTNode*> NodeRegistry;
        typedef std::multimap<SlotFlags , ASTSlotLink> EdgeRegistry;

 		Graph(ASTNodeFactory* factory);
		~Graph();

        // signals (can be connected)

        SIGNAL(on_reset);
        SIGNAL(on_change);
        SIGNAL(on_add   , ASTNode*);
        SIGNAL(on_remove, ASTNode*);

        // general

        bool                     update();
        void                     clear();  // Delete all nodes, wires, edges and reset scope.
        ASTScope*                   main_scope() { return m_root ? m_root->internal_scope() : nullptr; };
        inline GraphView*        view() const { return m_view; };
        inline void              set_view(GraphView* view = nullptr) { ASSERT(view != nullptr); m_view = view; }
        inline bool              is_empty() const { return m_root.empty(); };
        inline tools::Optional<ASTNode*> root() const { return m_root; }
        inline bool              is_root(const ASTNode* node) const { return m_root == node; }

        // node related

        ASTNode*                    create_node(); // Create a raw node.
        ASTNode*                    create_node(CreateNodeType, const tools::FunctionDescriptor* _signature = nullptr); // Create a given node type in a simple way.
        ASTNode*                    create_entry_point();
        ASTVariable*            create_variable(const tools::TypeDescriptor *_type, const std::string &_name);
        ASTVariableRef*         create_variable_ref();
        ASTVariable*            create_variable_decl(const tools::TypeDescriptor* _type, const char* _name);
        ASTLiteral*             create_literal(const tools::TypeDescriptor *_type);
        ASTFunctionCall*            create_function(const tools::FunctionDescriptor&);
        ASTFunctionCall*            create_operator(const tools::FunctionDescriptor&);
        ASTIf*                  create_cond_struct();
        ASTForLoop*             create_for_loop();
        ASTWhileLoop*           create_while_loop();
        ASTNode*                    create_empty_instruction();
        void                     destroy(ASTNode* _node);
        std::vector<ASTScope *>     scopes();
        std::set<ASTScope *>        root_scopes();
        NodeRegistry&            nodes() {return m_node_registry;}
        const NodeRegistry&      nodes()const {return m_node_registry;}
        void                     destroy_next_frame_ex(ASTNode* node, bool with_inputs );
        void                     destroy_next_frame(ASTNode* node) { destroy_next_frame_ex(node, false ); }
        void                     destroy_next_frame(ASTScope* scope);

        template<typename T> inline ASTVariable* create_variable_decl(const char*  _name = "var"){ return create_variable_decl(tools::type::get<T>(), _name); }
        template<typename T> inline ASTLiteral*  create_literal() { return create_literal(tools::type::get<T>()); }

        // edge related

        ASTSlotLink  connect(ASTNodeSlot* tail, ASTNodeSlot* head, ConnectFlags = ConnectFlag_NONE );
        void          connect(const std::set<ASTNodeSlot*>& tails, ASTNodeSlot* head, ConnectFlags _flags);
        ASTSlotLink  connect_to_variable(ASTNodeSlot* output_slot, ASTVariable* variable );
        ASTSlotLink  connect_or_merge(ASTNodeSlot* tail, ASTNodeSlot* head);
        void          disconnect(const ASTSlotLink& edge, ConnectFlags = ConnectFlag_NONE );
        EdgeRegistry& get_edge_registry() { return m_edge_registry; }

    private:
        void on_disconnect_value_side_effects(ASTSlotLink);
        void on_disconnect_flow_side_effects(ASTSlotLink);
        void on_connect_value_side_effects(ASTSlotLink);
        void on_connect_flow_side_effects(ASTSlotLink);

        // registries management
        void        add(ASTNode*);           // ... to the registry.
        void        remove(ASTNode*);        // ... from the registry.
        ASTSlotLink add(const ASTSlotLink&);    // ... to the registry.
        void        remove(const ASTSlotLink&); // ... from the registry.

        tools::Optional<ASTNode*> m_root;
        const ASTNodeFactory* m_factory  = nullptr;
        GraphView*         m_view     = nullptr; // non-owned
        NodeRegistry       m_node_registry;
        NodeRegistry       m_node_to_delete;
        EdgeRegistry       m_edge_registry;
    };
}