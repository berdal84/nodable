#include "Node.h"
#include "ForLoopNode.h"
#include "core/InstructionNode.h"
#include "core/Scope.h"

#include <utility>
#include <algorithm> // for std::find

#include "Graph.h"
#include "Scope.h"
#include "core/InvokableComponent.h"
#include "core/algorithm.h"


using namespace ndbl;
using fw::pool::Pool;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<Node>("Node");
}

Node::Node(std::string _label)
    : parent()
    , name(std::move(_label))
    , dirty(true)
    , flagged_to_delete(false)
    , props(m_id)
{
    FW_EXPECT( false, "TODO: declare edge count limits (on Properties?)" )
    // props.get_this().set_limits(0, 0) // 0 input, 0 output allowed
}

void Node::poolid(ID<Node> new_id)
{
    m_id = new_id;

    FW_EXPECT( props.has(THIS_PROPERTY) == false, "PropertyBag must have a \"this\" property." )
    get_prop(THIS_PROPERTY)->set( m_id );

    m_components.set_owner( m_id );

    // propagate events to then view
    auto redirect_event = [=](SlotBag::Event _event) -> void
    {
        m_id->on_slot_change.emit( _event );
    };
    slots.on_change.connect( redirect_event );
}

void Node::remove_edge(DirectedEdge edge)
{
    FW_EXPECT( false, "TODO: find edge in slots" )
    FW_EXPECT( false, "TODO: delete in slots" )
	//if( found )
    {
        dirty = true;
    }
}

size_t Node::incoming_edge_count()const
{
    return edge_count( Way::In );
}

size_t Node::outgoing_edge_count()const
{
    return edge_count( Way::Out );
}

size_t Node::edge_count(Way _way)const
{
    FW_EXPECT( false, "TODO: move implem to SlotBag, keep method on Node too" );
    // Get edge outbounds
    size_t count = 0;
    for(auto& slot : slots.data() )
    {
        if ( slot.allows(_way) )
        {
            count += slot.edge_count();
        }
    }
    return count;
}

const fw::iinvokable* Node::get_connected_invokable(const char* property_name) const
{
    ID<Property> property_id = props.get_id( property_name );

    // Find an edge connected to this node property
    DirectedEdge edge = get_edge_heading(property_id );

    // If found, we try to get the InvokableComponent from edge tail node.
    if (edge != DirectedEdge::null )
    {
        auto tail_node = edge.tail.node;
        if ( auto* invokable = tail_node->get_component<InvokableComponent>().get() )
        {
            return invokable->get_function();
        }
    }
    return nullptr;
}

bool Node::has_edge_heading(const char* _name) const
{
    ID<Property> id = props.get_id(_name);
    return has_edge_heading( id );
}

bool Node::has_edge_heading(ID<Property> property_id) const
{
    Slot& slot = get_slot( property_id, Way::In );
    auto is_heading_property = [property_id](auto& each_edge) { return each_edge.head.property == property_id; };
    auto it = std::find_if( slot.edges.begin(), slot.edges.end(), is_heading_property);
    return it != slot.edges.end();
}

void Node::set_parent(ID<Node> new_parent_id)
{
    FW_EXPECT(false, "Add/remove related edges in SlotBag");
    parent = new_parent_id;
    dirty = true;
}

void Node::set_name(const char *_label)
{
    name = _label;
    on_name_change.emit(m_id);
}

std::vector<PoolID<Component>> Node::get_components()
{
    return m_components.get_all();
}

DirectedEdge Node::get_edge_heading(const char* _name) const
{
    ID<Property> id = props.get_id( _name );
    return get_edge_heading( id );
}

DirectedEdge Node::get_edge_heading(ID<Property> property_id) const
{
    Slot& slot = get_slot(property_id, Way::In);

    auto is_heading_property = [=](const DirectedEdge& edge ) -> bool {
        return edge.head.property == property_id;
    };
    auto it = std::find_if(slot.edges.begin(), slot.edges.end(), is_heading_property);
    if ( it != slot.edges.end() )
    {
        return *it;
    }
    return DirectedEdge::null;
}

Slot& Node::get_slot(const char* property_name, Way desired_way) const
{
    const Property* property = get_prop(property_name);
    return get_slot(property->id, desired_way);
}

Slot& Node::get_slot(ID<Property> property_id, Way desired_way) const
{
    const Property* property = get_prop_at(property_id);
    FW_EXPECT( property->allows_connection( desired_way ), "This property does not allow this way of connection" );
    FW_EXPECT( false, "TODO: add a m_slot_bag to the class, find the right slot from property_id and desired_way");
}

std::vector<PoolID<Node>> Node::get_predecessors() const
{
    std::vector<PoolID<Node>> result;
    Slot& slot = get_slot(THIS_PROPERTY, Way::In);
    result.reserve(slot.edge_count());
    for (auto& edge : slot.edges ) result.push_back( edge.tail.node );
    return result;
}


std::vector<DirectedEdge> Node::get_input_edges(const std::vector<ID<Property>>& properties) const
{
    std::vector<DirectedEdge> result;
    result.reserve(properties.size());
    std::transform(properties.begin(), properties.end(),
                   result.end(),
                   [=](auto& each_id ) { return get_edge_heading(each_id); });
    return std::move(result);
}

const Property* Node::get_prop(const char *_name) const
{
    return props.get(_name);
}

Property* Node::get_prop(const char *_name)
{
    return props.get(_name);
}

const Property* Node::get_prop_at(ID<Property> id) const
{
    return props.at(id);
}

Property* Node::get_prop_at(ID<Property> id)
{
    return props.at(id);
}

bool Node::allows_more(Relation edge_type)
{
    return slots.allows_more( edge_type );
}

Slot& Node::slot(Way way) const
{
    return get_slot(THIS_PROPERTY, way);
}

std::vector<PoolID<Node>> Node::children() const
{
    std::vector<PoolID<Node>> result;
    for(const Slot& slot : slots.by_relation(Relation::CHILD_PARENT) )
    {
        for(const DirectedEdge& edge : slot.edges )
        {
            result.push_back( edge.tail.node );
        }
    }
    return result;
}

std::vector<DirectedEdge> Node::edges() const
{
    return slots.edges();
}

std::vector<PoolID<Node>> Node::successors() const
{
    std::vector<PoolID<Node>> result;

    for( auto& slot : slots.by_relation(Relation::NEXT_PREVIOUS) )
    {
        for( auto& edge : slot.edges )
        {
            result.push_back( edge.head.node );
        }
    }

    return result;
}

std::vector<DirectedEdge> Node::filter_edges(Relation type) const
{
    std::vector<DirectedEdge> result;
    for (auto& slot: slots.by_relation(type))
    {
        for (auto& edge: slot.edges)
        {
            result.push_back( edge );
        }
    }
    return result;
}

size_t Node::get_slot_count(Relation edge_type, Way way) const
{
    return slots.count(edge_type, way);
}

Slot& Node::get_slot(Property* property, Way way) const
{
    return slots.by_property(property, way);
}

void Node::add_edge(DirectedEdge)
{
    FW_EXPECT(false, "TODO: add the edge into the slots, handle side effects?")
}
