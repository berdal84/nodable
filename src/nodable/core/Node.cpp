#include "Node.h"

#include <utility>
#include <algorithm> // for std::find

#include "core/InvokableComponent.h"

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<Node>("Node");
}

Node::Node(std::string _label)
    : successors(0)
    , predecessors(0)
    , props(this)
    , parent_graph(nullptr)
    , parent(nullptr)
    , name(std::move(_label))
    , dirty(true)
    , flagged_to_delete(false)
    , components(this)
{
    /*
     * Add "this" Property to be able to connect this Node as an object pointer.
     * Usually an object pointer is connected to an InstructionNode's "node_to_eval" Property.
     */
    auto property = props.add<Node*>(k_this_property_name, Visibility::Always, Way::Way_Out);
    property->set(this);
    this->as_property = property.get();

    // propagate "inputs" events
    inputs.m_on_added.connect( [this](Node* _node){
        on_edge_added.emit(_node, Edge_t::IS_INPUT_OF);
        dirty = true;
    });

    inputs.m_on_removed.connect( [this](Node* _node){
        on_edge_removed.emit(_node, Edge_t::IS_INPUT_OF);
        dirty = true;
    });

    // propagate "outputs" events
    outputs.m_on_added.connect( [this](Node* _node){
        on_edge_added.emit(_node, Edge_t::IS_OUTPUT_OF);
        dirty = true;
    });

    outputs.m_on_removed.connect( [this](Node* _node){
        on_edge_removed.emit(_node, Edge_t::IS_OUTPUT_OF);
        dirty = true;
    });

    // propagate "children" events
    children.m_on_added.connect( [this](Node* _node){
        on_edge_added.emit(_node, Edge_t::IS_CHILD_OF);
        dirty = true;
    });

    children.m_on_removed.connect( [this](Node* _node){
        on_edge_removed.emit(_node, Edge_t::IS_CHILD_OF);
        dirty = true;
    });
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
    return std::count_if(edges.cbegin(), edges.cend()
                       , [this](const auto each_edge) { return each_edge->prop.dst->get_owner() == this; });
}

size_t Node::outgoing_edge_count()const
{
	return std::count_if(edges.cbegin(), edges.cend()
                       , [this](const auto each_edge) { return each_edge->prop.src->get_owner() == this; });
}

const fw::iinvokable* Node::get_connected_invokable(const Property* _local_property)
{
    FW_EXPECT(_local_property->get_owner() == this, "This node has no property with this address!");

    // Find an edge connected to _property
    auto found = std::find_if(edges.cbegin(), edges.cend(), [_local_property](const DirectedEdge* each_edge)->bool {
        return each_edge->prop.dst == _local_property;
    });

    // If found, we try to get the InvokableComponent from its source node.
    if (found != edges.end() )
    {
        Node* node = (*found)->prop.src->get_owner();
        InvokableComponent* compute_component = node->components.get<InvokableComponent>();
        if ( compute_component )
        {
            return compute_component->get_function();
        }
    }

    return nullptr;
}

bool Node::is_connected_with(const Property *_localProperty)
{
    /*
     * Find a wire connected to _property
     */
    auto found = std::find_if(edges.cbegin(), edges.cend(), [_localProperty](const DirectedEdge* _each_edge)->bool {
        return _each_edge->prop.dst == _localProperty;
    });

    return found != edges.end();
}

void Node::set_parent(Node *_node)
{
    FW_ASSERT(_node != nullptr || parent != nullptr);
    parent = _node;
    dirty = true;
}

void Node::set_name(const char *_label)
{
    name = _label;
    on_name_change.emit(this);
}

void Node::add_edge(const DirectedEdge *edge)
{
    edges.insert(edge);
    dirty = true;
}
