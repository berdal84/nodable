#include "Graph.h"

#include <algorithm>    // std::find_if

#include "DirectedEdge.h"
#include "IfNode.h"
#include "LiteralNode.h"
#include "Node.h"
#include "NodeFactory.h"
#include "Scope.h"
#include "VariableNode.h"
#include "language/Nodlang.h"
#include "VariableRefNode.h"

using namespace ndbl;
using namespace tools;

Graph::Graph(NodeFactory* factory)
: m_factory(factory)
{}

Graph::~Graph()
{
	clear();
}

void Graph::clear()
{
    if ( m_root == nullptr && m_node_registry.empty() && m_edge_registry.empty() )
    {
        return;
    }

	LOG_VERBOSE( "Graph", "Clearing graph ...\n");
    m_root = nullptr;
    while ( !m_node_registry.empty() )
    {
        Node* node = m_node_registry[0];
        LOG_VERBOSE("Graph", "destroying node \"%s\" (id: %zu)\n", node->name().c_str(), (u64_t)node );
        destroy(node);
    }

#ifdef NDBL_DEBUG
    if ( !m_edge_registry.empty() )
    {
        LOG_ERROR("Graph", "m_edge_registry should be empty.\n" );
        LOG_MESSAGE("Graph", "Dumping %zu edge(s) for debugging purpose ...\n", m_edge_registry.size() );
        for ( auto& edge : m_edge_registry)
        {
            LOG_MESSAGE("Graph", "   %s\n", to_string(edge.second).c_str() );
        }
        m_edge_registry.clear();
    }
#endif

    LOG_VERBOSE("Graph", "Graph cleared.\n");
}

bool Graph::update()
{
    if ( !m_is_dirty )
        return false;

    bool changed = false;

    // Delete (flagged Nodes) / Check if dirty
    auto nodeIndex = m_node_registry.size();

    while (nodeIndex > 0)
    {
        nodeIndex--;
        Node* node = m_node_registry.at(nodeIndex);

        if (node->has_flags(NodeFlag_TO_DELETE))
        {
            destroy(node);
            changed = true;
        }
        else if ( node->has_flags(NodeFlag_IS_DIRTY) )
        {
            changed = node->update();
        }
    }

    if ( changed )
        on_update.emit();

    return changed;
}

void Graph::add(Node* _node)
{
    ASSERT(std::find(m_node_registry.begin(), m_node_registry.end(), _node) == m_node_registry.end());

	m_node_registry.push_back(_node);
    _node->m_graph = this;

    on_add.emit(_node);

    set_dirty();

    LOG_VERBOSE("Graph", "add node %s (%s)\n", _node->name().c_str(), _node->get_class()->get_name());
}

void Graph::remove(Node* _node)
{
    auto found = std::find(m_node_registry.begin(), m_node_registry.end(), _node);
    ASSERT(found != m_node_registry.end());
    m_node_registry.erase(found);

    on_remove.emit(_node);

    set_dirty();
}

void Graph::ensure_has_root()
{
    if( is_empty() )
    {
        create_root();
    }
}

VariableNode* Graph::create_variable(const TypeDescriptor *_type, const std::string& _name, Scope* _scope)
{
    VariableNode* node = m_factory->create_variable(_type, _name, _scope);
    add(node);
	return node;
}

FunctionNode* Graph::create_function(const FunctionDescriptor* _type)
{
    FunctionNode* node = m_factory->create_function(_type, NodeType_FUNCTION);
    add(node);
    return node;
}

FunctionNode* Graph::create_operator(const FunctionDescriptor* _type)
{
    FunctionNode* node = m_factory->create_function(_type, NodeType_OPERATOR);
    add(node);
    return node;
}

void Graph::destroy(Node* node)
{
    if( node == nullptr )
    {
        return;
    }

    // Identify each edge connected to this node
    std::vector<DirectedEdge> related_edges;
    for(auto& [_type, each_edge]: m_edge_registry)
    {
        if(each_edge.tail->node == node || each_edge.head->node == node )
        {
            related_edges.emplace_back(each_edge);
        }
    }
    // Disconnect all of them
    for(const DirectedEdge& each_edge : related_edges )
    {
        disconnect(each_edge);
    };

    // if it is a variable, we remove it from its scope
    if ( node->type() == NodeType_VARIABLE )
    {
        VariableNode* node_variable = cast<VariableNode>(node);
        if ( IScope* scope = node_variable->get_scope() )
        {
            scope->remove_variable(node_variable);
        }
    }
    else if ( auto* scope = node->get_component<Scope>() )
    {
        if ( !scope->has_no_variable() )
        {
            scope->remove_all_variables();
        }
    }

    // unregister and delete
    remove(node);
    if ( m_root == node )
    {
        m_root = {};
    }
    m_factory->destroy_node(node);
}

bool Graph::is_empty() const
{
    return m_root == nullptr;
}

DirectedEdge* Graph::connect_or_merge(Slot&_out, Slot& _in )
{
    // Guards
    ASSERT( _in.has_flags( SlotFlag_INPUT ) );
    ASSERT( _in.has_flags( SlotFlag_NOT_FULL ) );
    ASSERT( _out.has_flags( SlotFlag_OUTPUT ) );
    ASSERT( _out.has_flags( SlotFlag_NOT_FULL ) );
    VERIFY(_in.property, "tail get_value must be defined" );
    VERIFY(_out.property, "head get_value must be defined" );
    VERIFY(_in.property != _out.property, "Can't connect same properties!" );

    // now graph is abstract
//    const type* out_type = __out.property->get_type();
//    const type* in_type  = _in.property->get_type();
//    EXPECT( type::is_implicitly_convertible( out_type, in_type ), "dependency type should be implicitly convertible to dependent type");

    // case 1: merge orphan slot
    if (_out.node == nullptr ) // if dependent is orphan
    {
        _in.property->digest( _out.property );
        delete _in.property;
        // set_dirty(); // no changes on edges/nodes
        return nullptr;
    }

    // case 2: merge literals when not connected to a variable
    if (_out.node->type() == NodeType_LITERAL )
        if (_in.node->type() != NodeType_VARIABLE )
        {
            _in.property->digest( _out.property );
            destroy(_out.node);
            set_dirty(); // a node has been destroyed
            return nullptr;
        }

    // Connect (case 4)
    set_dirty();
    return connect( _out, _in, ConnectFlag_ALLOW_SIDE_EFFECTS );
}

void Graph::remove(DirectedEdge edge)
{
    auto found = std::find_if( m_edge_registry.begin()
                             , m_edge_registry.end()
                             , [edge](auto& each){ return edge == each.second;});

    if (found != m_edge_registry.end() )
    {
        m_edge_registry.erase(found);
    }
    else
    {
        LOG_WARNING("Graph", "Unable to unregister edge\n");
    }
    set_dirty(); // To express this graph changed
}

DirectedEdge* Graph::connect_to_variable(Slot& _out, VariableNode& _variable )
{
    // Guards
    ASSERT( _out.has_flags( SlotFlag_OUTPUT | SlotFlag_NOT_FULL ));
    return connect_or_merge( _out, *_variable.value_in() );
}

DirectedEdge* Graph::connect(Slot& _first, Slot& _second, ConnectFlags _flags)
{
    ASSERT( _first.has_flags( SlotFlag_ORDER_FIRST ) );
    ASSERT( _second.has_flags( SlotFlag_ORDER_SECOND ) );
    ASSERT(_first.node != _second.node );

    // Insert edge
    SlotFlags type = _first.type();

    auto& [_, edge] = *m_edge_registry.emplace( type, DirectedEdge{&_first, &_second});

    // Add cross-references to each end of the edge
    Slot *tail = edge.tail;
    Slot *head = edge.head;
    ASSERT(tail != head);
    ASSERT(head != nullptr);
    ASSERT(tail != nullptr);

    tail->add_adjacent(head);
    head->add_adjacent(tail);

    // Handle side effects
    if (_flags & ConnectFlag_ALLOW_SIDE_EFFECTS )
    {
        switch ( type )
        {
            case SlotFlag_TYPE_HIERARCHICAL:
            {
                // Ensure to Identify parent and child nodes
                // - parent node has a CHILD slot
                // - child node has a PARENT slot
                Node* parent    = _first.node;  static_assert(SlotFlag_CHILD & SlotFlag_ORDER_FIRST);
                Node* new_child = _second.node; static_assert(SlotFlag_PARENT & SlotFlag_ORDER_SECOND);
                ASSERT( parent->has_component<Scope>());

                Slot* parent_next_slot    = parent->find_slot_at( SlotFlag_NEXT, _first.position );
                ASSERT(parent_next_slot);
                Slot& new_child_prev_slot = *new_child->find_slot( SlotFlag_PREV );

                // Case 1: Parent has only 1 child (the newly connected), we connect it as "next".
                if ( !parent_next_slot->is_full() )
                {
                    connect( *parent_next_slot, new_child_prev_slot );
                }
                // Case 2: Connects to the last child's "next" slot.
                //         parent
                //           - ...
                //           - last child ->->->
                //           - new child <-<-<-
                else
                {
                    Node* previous_child = *(parent->children().rbegin() + 1);
                    ASSERT( previous_child );

                    // Case 2.a: Connects to all last instructions' "next" slot (in last child's previous_child_scope).
                    //           parent
                    //             - ...
                    //             - previous_child
                    //                  - child 0
                    //                     - ...
                    //                     - instr n >->->->->
                    //                  - child 1
                    //                     - instr 0 >->->->->
                    //                  - ...
                    //                  - instr n ->->->->->->
                    //             - new child <-<-<-<-<-<-<-<
                    //
                    if ( previous_child->has_component<Scope>() )
                    {
                        auto previous_child_scope = previous_child->get_component<Scope>();
                        for (Node* each_instr : previous_child_scope->get_last_instructions_rec() )
                        {
                            Slot* each_instr_next_slot = each_instr->find_slot( SlotFlag_NEXT | SlotFlag_NOT_FULL );
                            ASSERT(each_instr_next_slot);
                            connect( *each_instr_next_slot, new_child_prev_slot );
                        }
                    }
                    // Case 2.b: Connects to last child's "next" slot.
                    //           parent
                    //             - ...
                    //             - previous_child ->->->
                    //             - new child <-<-<-<-<-<
                    //
                    else
                    {
                        Slot* last_sibling_next_slot = previous_child->find_slot( SlotFlag_NEXT | SlotFlag_NOT_FULL );
                        connect( *last_sibling_next_slot, new_child_prev_slot );
                    }
                }
                break;
            }

            case SlotFlag_TYPE_CODEFLOW:
            {
                Node& prev_node = *_first.node; static_assert(SlotFlag_NEXT & SlotFlag_ORDER_FIRST );
                Node& next_node = *_second.node; static_assert(SlotFlag_PREV & SlotFlag_ORDER_SECOND );

                // If previous node is a scope, connects next_node as child
                if ( prev_node.has_component<Scope>() )
                {
                    connect(
                            *prev_node.find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
                            *next_node.find_slot( SlotFlag_PARENT ));
                }
                // If next node parent exists, connects next_node as a child too
                else if ( Node* prev_parent_node = prev_node.parent() )
                {
                    connect(
                            *prev_parent_node->find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
                            *next_node.find_slot( SlotFlag_PARENT ));
                }

                // Connect siblings
                else if ( Node* prev_parent_node = prev_node.parent() )
                {
                    Node* current_prev_node_sibling = prev_node.successors()[0];
                    while ( current_prev_node_sibling && current_prev_node_sibling->parent() )
                    {
                        connect(
                                *current_prev_node_sibling->find_slot( SlotFlag_CHILD | SlotFlag_NOT_FULL ),
                                *prev_parent_node->find_slot( SlotFlag_PARENT ) );
                        current_prev_node_sibling = *current_prev_node_sibling->successors().begin();
                    }
                }
                break;
            }
            case SlotFlag_TYPE_VALUE:
                // Nothing to do in such case
                break;
            default:
                ASSERT(false);// This connection type is not yet implemented
        }
    }
    set_dirty(); // To express this graph changed
    return &edge;
}

void Graph::disconnect( const DirectedEdge& _edge, ConnectFlags flags)
{
    // find the edge to disconnect
    SlotFlags type = _edge.tail->flags() & SlotFlag_TYPE_MASK;
    auto [range_begin, range_end]   = m_edge_registry.equal_range(type);
    auto it = std::find_if( range_begin, range_end, [&](const auto& _pair) -> bool { return _edge == _pair.second; });
    VERIFY(it != m_edge_registry.end(), "Unable to find edge" );

    // erase it from the registry
    m_edge_registry.erase(it);

    // disconnect the slots
    Slot *tail = _edge.head;
    Slot *head = _edge.tail;
    ASSERT(tail != head);
    ASSERT(head != nullptr);
    ASSERT(tail != nullptr);

    tail->remove_adjacent(head);
    head->remove_adjacent(tail);

    // disconnect effectively
    switch ( type )
    {
        case SlotFlag_TYPE_CODEFLOW:
        {
            ASSERT(_edge.head->has_flags(SlotFlag_PREV));
            Node* next = _edge.head->node;
            Node* next_parent = next->parent();
            if ( flags & ConnectFlag_ALLOW_SIDE_EFFECTS && next_parent )
            {
                while ( next && next_parent == next->parent() )
                {
                    disconnect({
                            next->parent()->find_slot( SlotFlag_CHILD ),
                            next->find_slot( SlotFlag_PARENT )
                    });

                    std::vector<Node*> successors = next->successors();
                    next = successors.begin() != successors.end() ? *successors.begin()
                                                                  : nullptr;
                }
            }

            break;
        }

        case SlotFlag_TYPE_HIERARCHICAL:
        case SlotFlag_TYPE_VALUE:
            // None
            break;
        default:
            VERIFY(!type, "Not yet implemented yet");
    }

    set_dirty(); // To express this graph changed
}

Node* Graph::create_scope()
{
    Node* scopeNode = m_factory->create_scope();
    add(scopeNode);
    return scopeNode;
}

IfNode* Graph::create_cond_struct()
{
    IfNode* condStructNode = m_factory->create_cond_struct();
    add(condStructNode);
    return condStructNode;
}

ForLoopNode* Graph::create_for_loop()
{
    ForLoopNode* for_loop = m_factory->create_for_loop();
    add(for_loop);
    return for_loop;
}

WhileLoopNode* Graph::create_while_loop()
{
    WhileLoopNode* while_loop = m_factory->create_while_loop();
    add(while_loop);
    return while_loop;
}

Node* Graph::create_root()
{
    Node* node = m_factory->create_program();
    add(node);
    m_root = node;
    return node;
}

Node* Graph::create_node()
{
    Node* node = m_factory->create_node();
    add(node);
    return node;
}

LiteralNode* Graph::create_literal(const TypeDescriptor *_type)
{
    LiteralNode* node = m_factory->create_literal(_type);
    add(node);
    return node;
}

Node* Graph::create_node( CreateNodeType _type, const FunctionDescriptor* _signature )
{
    switch ( _type )
    {
        /*
         * TODO: We could consider narowing the enum to few cases (BLOCK, VARIABLE, LITERAL, OPERATOR, FUNCTION)
         *       and rely more on _signature (ex: a bool variable could be simply "bool" or "bool bool(bool)")
         */
        case CreateNodeType_BLOCK_CONDITION:  return create_cond_struct();
        case CreateNodeType_BLOCK_FOR_LOOP:   return create_for_loop();
        case CreateNodeType_BLOCK_WHILE_LOOP: return create_while_loop();
        case CreateNodeType_BLOCK_SCOPE:      return create_scope();
        case CreateNodeType_BLOCK_PROGRAM:    clear(); return create_root();

        case CreateNodeType_VARIABLE_BOOLEAN: return create_variable_decl<bool>();
        case CreateNodeType_VARIABLE_DOUBLE:  return create_variable_decl<double>();
        case CreateNodeType_VARIABLE_INTEGER: return create_variable_decl<int>();
        case CreateNodeType_VARIABLE_STRING:  return create_variable_decl<std::string>();

        case CreateNodeType_LITERAL_BOOLEAN:  return create_literal<bool>();
        case CreateNodeType_LITERAL_DOUBLE:   return create_literal<double>();
        case CreateNodeType_LITERAL_INTEGER:  return create_literal<int>();
        case CreateNodeType_LITERAL_STRING:   return create_literal<std::string>();

        case CreateNodeType_FUNCTION:
        {
            VERIFY(_signature != nullptr, "_signature is expected when dealing with functions or operators");
            Nodlang* language = get_language();
            // Currently, we handle operators and functions the exact same way
            const FunctionDescriptor* signature = language->find_function(_signature)->get_sig();
            bool is_operator = language->find_operator_fct( signature ) != nullptr;
            if ( is_operator )
                return create_operator(signature);
            return create_function(signature);
        }
        default:
            VERIFY(false, "Unhandled CreateNodeType.");
            return nullptr;
    }
}

VariableRefNode* Graph::create_variable_ref()
{
    VariableRefNode* node = m_factory->create_variable_ref();
    add(node);
    return node;
}

VariableNode* Graph::create_variable_decl(const TypeDescriptor* _type, const char*  _name, Scope*  _scope)
{
    if( !_scope)
    {
        _scope = get_root()->get_component<Scope>();
    }

    // Create variable
    VariableNode* var_node = create_variable(_type, _name, _scope );
    var_node->set_flags(VariableFlag_DECLARED); // yes, when created from the graph view, variables can be undeclared (== no scope).
    Token token(Token_t::keyword_operator, " = ");
    token.m_word_start_pos = 1;
    token.m_word_length = 1;
    var_node->set_operator_token( token );

    // TODO: attach a default Literal?

    return var_node;
}

void Graph::set_view(ndbl::GraphView* view)
{
    ASSERT(view != nullptr);
    m_view = view;
}
