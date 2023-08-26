#include "Node.h"

#include <utility>
#include <algorithm> // for std::find

#include "core/InvokableComponent.h"
#include "Scope.h"
#include "Graph.h"

using namespace ndbl;
using fw::pool::Pool;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<Node>("Node");
}

Node::Node(std::string _label)
    : successors( Edge_t::IS_SUCCESSOR_OF, 0)
    , predecessors( Edge_t::IS_PREDECESSOR_OF, 0)
    , inputs(Edge_t::IS_INPUT_OF, Slots_LIMIT_MAX )
    , outputs(Edge_t::IS_OUTPUT_OF, Slots_LIMIT_MAX )
    , children(Edge_t::IS_CHILD_OF, Slots_LIMIT_MAX )
    , parent()
    , name(std::move(_label))
    , dirty(true)
    , flagged_to_delete(false)
{
}

void Node::id(ID<Node> new_id)
{
    m_id = new_id;
    props.set_owner( m_id );
    m_components.set_owner( m_id );

    // propagate "inputs" events
    auto redirect_event = [new_id](ID<Node> _node, SlotEvent _event, Edge_t _edge_type) -> void
    {
        new_id->on_slot_change.emit( _node, _event, _edge_type );
    };

    inputs.on_change.connect( redirect_event );
    outputs.on_change.connect( redirect_event );
    children.on_change.connect( redirect_event );
    predecessors.on_change.connect( redirect_event );
    successors.on_change.connect( redirect_event );
}

void Node::remove_edge(const DirectedEdge*edge)
{
	auto found = edges.find(edge);
	if(found != edges.end())
    {
        edges.erase(found);
        dirty = true;
    }
}

size_t Node::incoming_edge_count()const
{
    // Get edge outbounds
    size_t count = 0;
    std::for_each(
        edges.cbegin(),
        edges.cend(),
        [this, &count](const DirectedEdge* each_edge)
        {
            auto [_, dest] = each_edge->nodes();
            if( dest == this ) count++;
        }
    );
    return count;
}

size_t Node::outgoing_edge_count()const
{
    // Get edge outbounds
    size_t count = 0;
    std::for_each(
            edges.cbegin(),
            edges.cend(),
            [this, &count](const DirectedEdge* each_edge)
            {
                auto [src, _] = each_edge->nodes();
                if( src == this ) count++;
            }
    );
    return count;
}

const fw::iinvokable* Node::get_connected_invokable(const Property* _local_property) const
{
    FW_EXPECT(_local_property->owner() == m_id, "This node has no property with this address!");

    // Find an edge connected to _property
    auto found = std::find_if(edges.cbegin(), edges.cend(), [_local_property](const DirectedEdge* each_edge)->bool {
        return each_edge->dst() == _local_property;
    });

    // If found, we try to get the InvokableComponent from its source node.
    if (found != edges.end() )
    {
        auto src_node = (*found)->src_node();
        if ( auto* invokable = src_node->get_component<InvokableComponent>().get() )
        {
            return invokable->get_function();
        }
    }

    return nullptr;
}

bool Node::is_connected_with(const Property* property)
{
    /*
     * Find a wire connected to _property
     */
    for(auto each_edge : edges )
    {
        if( each_edge->dst() == property )
        {
            return true;
        }
    }
    return false;
}

void Node::set_parent(ID<Node> new_parent_id)
{
    parent = new_parent_id;
    dirty = true;
}

void Node::set_name(const char *_label)
{
    name = _label;
    on_name_change.emit(m_id);
}

void Node::add_edge(const DirectedEdge *edge)
{
    edges.insert(edge);
    dirty = true;
}

std::vector<ID<Component>> Node::get_components()
{
    return m_components.get_all();
}