#include <ndbl/core/Node.h>
#include <utility>
#include <algorithm> // for std::find
#include <fw/core/log.h>
#include <fw/core/reflection/func_type.h>
#include <fw/core/reflection/invokable.h>
#include <ndbl/core/DataAccess.h>
#include <ndbl/core/InvokableComponent.h>

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    fw::registration::push_class<Node>("Node");
}

Node::Node(std::string _label)
    : m_successors(0)
    , m_predecessors(0)
    , m_props(this)
    , m_parent_graph(nullptr)
    , m_parent(nullptr)
    , m_name(std::move(_label))
    , m_inner_graph(nullptr)
    , m_dirty(true)
    , m_flagged_to_delete(false)
    , m_components(this)
{
    /*
     * Add "this" Property to be able to connect this Node as an object pointer.
     * Usually an object pointer is connected to an InstructionNode's "node_to_eval" Property.
     */
    auto this_property = m_props.add<Node*>(k_this_property_name, Visibility::Always, Way::Way_Out);
    this_property->set( this );

    // propagate "inputs" events
    m_inputs.m_on_added.connect( [this](Node* _node){
        on_edge_added.emit(_node, Edge_t::IS_INPUT_OF);
        set_dirty();
    });

    m_inputs.m_on_removed.connect( [this](Node* _node){
        on_edge_removed.emit(_node, Edge_t::IS_INPUT_OF);
        set_dirty();
    });


    // propagate "outputs" events
    m_outputs.m_on_added.connect( [this](Node* _node){
        on_edge_added.emit(_node, Edge_t::IS_OUTPUT_OF);
        set_dirty();
    });

    m_outputs.m_on_removed.connect( [this](Node* _node){
        on_edge_removed.emit(_node, Edge_t::IS_OUTPUT_OF);
        set_dirty();
    });

    // propagate "children" events
    m_children.m_on_added.connect( [this](Node* _node){
        on_edge_added.emit(_node, Edge_t::IS_CHILD_OF);
        set_dirty();
    });

    m_children.m_on_removed.connect( [this](Node* _node){
        on_edge_removed.emit(_node, Edge_t::IS_CHILD_OF);
        set_dirty();
    });
}

void Node::remove_edge(const DirectedEdge*edge)
{
	auto found = m_edges.find(edge);
	if(found != m_edges.end())
    {
        m_edges.erase(found);
        m_dirty = true;
    }
}

size_t Node::incoming_edge_count()const
{
    return std::count_if(m_edges.cbegin(), m_edges.cend()
                       , [this](const auto each_edge) { return each_edge->prop.dst->get_owner() == this; });
}

size_t Node::outgoing_edge_count()const
{
	return std::count_if(m_edges.cbegin(), m_edges.cend()
                       , [this](const auto each_edge) { return each_edge->prop.src->get_owner() == this; });
}

const fw::iinvokable* Node::get_connected_invokable(const Property* _local_property)
{
    FW_EXPECT(_local_property->get_owner() == this, "This node has no property with this address!");

    // Find an edge connected to _property
    auto found = std::find_if(m_edges.cbegin(), m_edges.cend(), [_local_property](const DirectedEdge* each_edge)->bool {
        return each_edge->prop.dst == _local_property;
    });

    // If found, we try to get the InvokableComponent from its source node.
    if (found != m_edges.end() )
    {
        Node* node = (*found)->prop.src->get_owner();
        InvokableComponent* compute_component = node->get_component<InvokableComponent>();
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
    auto found = std::find_if(m_edges.cbegin(), m_edges.cend(), [_localProperty](const DirectedEdge* _each_edge)->bool {
        return _each_edge->prop.dst == _localProperty;
    });

    return found != m_edges.end();
}
