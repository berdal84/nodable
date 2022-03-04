#pragma once

#include <string>
#include <vector>
#include <memory> // std::shared_ptr

namespace Nodable
{
    // forward declaration
    class InstructionNode;
    class ConditionalStructNode;
    class LiteralNode;
    class INodeFactory;

    typedef std::pair<const Relation_t, std::pair<Node*, Node*>> Relation;
    typedef std::multimap<Relation::first_type , Relation::second_type> RelationRegistry;

    inline bool operator==(
            const Relation& _left,
            const Relation& _right)
    {
        return (_left.first == _right.first) && (_left.second == _right.second);
    }

    /**
     * @brief a GraphNode is a context for a set of Nodes and Wires. It is also used to connect Nodes and Members.
     */
	class IGraph
	{
	public:
        virtual UpdateResult            update() = 0;
		virtual void                    clear() = 0;
        virtual std::vector<Node*>&     get_node_registry() =  0;
        virtual std::vector<Wire*>&     get_wire_registry() = 0;
        virtual const Language*         get_language()const = 0;
        virtual Node*                   get_root() = 0;
        virtual RelationRegistry&       get_relation_registry() = 0;
        virtual bool                    is_empty() = 0;
        virtual void                    ensure_has_root() = 0;
        virtual Node*                   create_root() = 0;
        virtual InstructionNode*        create_instr() = 0;
		virtual VariableNode*           create_variable(std::shared_ptr<const R::Type>, const std::string&, IScope*) = 0;
		virtual LiteralNode*            create_literal(std::shared_ptr<const R::Type>) = 0;
		virtual Node*                   create_bin_op(const InvokableOperator*) = 0;
		virtual Node*                   create_unary_op(const InvokableOperator*)  = 0;
        virtual Node*                   create_operator(const InvokableOperator*)  = 0;
		virtual Wire*                   create_wire() = 0;
		virtual Node*                   create_function(const IInvokable*) = 0;
        virtual Node*                   create_scope() = 0;
        virtual ConditionalStructNode*  create_cond_struct() = 0;
        virtual ForLoopNode*            create_for_loop() = 0;
        virtual Node*                   create_node() = 0;
        virtual Wire* connect(Member* _src, Member* _dst, ConnBy_ _connect_by = ConnectBy_Ref ) = 0;
        virtual void  connect(Node* _src, Node* _dst, Relation_t, bool _side_effects = true) = 0;
        virtual void  connect(Node* _src, InstructionNode* _dst) = 0;
        virtual void  connect(Member* _src, VariableNode* _dst)  = 0;
        virtual void  disconnect(Node* _src, Node* _dst, Relation_t, bool _side_effects = true) = 0;
        virtual void  disconnect(Wire*) = 0;
        virtual void  disconnect(Member* _member, Way _way = Way_InOut) = 0;
        virtual void  destroy(Node*) = 0;
    private:
        virtual void add(Node*) = 0;
        virtual void remove(Node*) = 0;
        virtual void add(Wire*) = 0;
        virtual void remove(Wire*) = 0;
        virtual void destroy(Wire*) = 0;
    };
}