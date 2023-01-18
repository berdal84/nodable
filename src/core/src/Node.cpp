#include <algorithm>    // for std::find
#include <utility>
#include <nodable/core/types.h>
#include <nodable/core/Node.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <nodable/core/DataAccess.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/reflection/func_type.h>

using namespace ndbl;

REGISTER
{
    using namespace ndbl;
    registration::push_class<Node>("Node");
}

Node::Node(std::string _label)
    : m_successors(0)
    , m_predecessors(0)
    , m_inputs()
    , m_outputs()
    , m_children()
    , m_props(this)
    , m_parent_graph(nullptr)
    , m_parent(nullptr)
    , m_name(std::move(_label))
    , m_inner_graph(nullptr)
    , m_dirty(true)
    , m_flagged_to_delete(false)
{
    /*
     * Add "this" Property to be able to connect this Node as an object pointer.
     * Usually an object pointer is connected to an InstructionNode's "node_to_eval" Property.
     */
    auto this_property = m_props.add<Node*>(k_this_property_name, Visibility::Always, Way::Way_Out);
    this_property->set( this );

    // propagate "inputs" events
    m_inputs.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, Edge_t::IS_INPUT_OF);
        set_dirty();
    });

    m_inputs.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, Edge_t::IS_INPUT_OF);
        set_dirty();
    });


    // propagate "outputs" events
    m_outputs.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, Edge_t::IS_OUTPUT_OF);
        set_dirty();
    });

    m_outputs.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, Edge_t::IS_OUTPUT_OF);
        set_dirty();
    });

    // propagate "children" events
    m_children.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, Edge_t::IS_CHILD_OF);
        set_dirty();
    });

    m_children.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, Edge_t::IS_CHILD_OF);
        set_dirty();
    });
}

bool Node::is_dirty()const
{
    return m_dirty;
}

void Node::set_dirty(bool _value)
{
    m_dirty = _value;
}

void Node::set_name(const char *_label)
{
    m_name = _label;
}

const char* Node::get_name()const
{
	return m_name.c_str();
}

void Node::add_edge(const DirectedEdge* edge)
{
    m_edges.insert(edge);
    m_dirty = true;
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

UpdateResult Node::update()
{
	if(has<DataAccess>())
    {
        get<DataAccess>()->update();
    }

    m_dirty = false;

	return UpdateResult::Success;
}

const iinvokable* Node::get_connected_invokable(const Property* _local_property)
{
    NDBL_EXPECT(_local_property->get_owner() == this, "This node has no property with this address!");

    // Find an edge connected to _property
    auto found = std::find_if(m_edges.cbegin(), m_edges.cend(), [_local_property](const DirectedEdge* each_edge)->bool {
        return each_edge->prop.dst == _local_property;
    });

    // If found, we try to get the InvokableComponent from its source node.
    if (found != m_edges.end() )
    {
        Node* node = (*found)->prop.src->get_owner();
        auto* compute_component = node->get<InvokableComponent>();
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

void Node::set_parent(Node *_node)
{
    NDBL_ASSERT(_node != nullptr || this->m_parent != nullptr);
    this->m_parent = _node;
    set_dirty();
}

void Node::set_parent_graph(GraphNode *_parentGraph)
{
    NDBL_ASSERT(this->m_parent_graph == nullptr); // TODO: implement parentGraph switch
    this->m_parent_graph = _parentGraph;
}

Node::~Node()
{
    if ( !m_components.empty())
        delete_components();
}

size_t Node::delete_components()
{
    size_t count(m_components.size());
    for ( const auto& keyComponentPair : m_components)
    {
        delete keyComponentPair.second;
    }
    m_components.clear();
    return count;
}
