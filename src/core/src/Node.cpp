#include <algorithm>    // for std::find
#include <utility>
#include <nodable/core/types.h>
#include <nodable/core/Node.h>
#include <nodable/core/Log.h> // for LOG_DEBUG(...)
#include <nodable/core/Wire.h>
#include <nodable/core/DataAccess.h>
#include <nodable/core/InvokableComponent.h>
#include <nodable/core/IInvokable.h>
#include <nodable/core/Signature.h>

using namespace Nodable;
using namespace Nodable::R;

R_DEFINE_CLASS(Node)

Node::Node(std::string _label)
    : m_successors(this, 0)
    , m_predecessors(this, 0)
    , m_inputs(this)
    , m_outputs(this)
    , m_children(this)
    , m_props(this)
    , m_parent_graph(nullptr)
    , m_parent(nullptr)
    , m_label(std::move(_label))
    , m_inner_graph(nullptr)
    , m_dirty(true)
    , m_needs_to_be_deleted(false)
{
    /*
     * Add "this" Member to be able to connect this Node as an object pointer.
     * Usually an object pointer is connected to an InstructionNode's "node_to_eval" Member.
     */
    Member* this_member = m_props.add<Node*>(k_this_member_name, Visibility::Always, Way::Way_Out);
    this_member->set( this );

    // propagate "inputs" events
    m_inputs.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, EdgeType::IS_INPUT_OF);
        set_dirty();
    });

    m_inputs.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, EdgeType::IS_INPUT_OF);
        set_dirty();
    });


    // propagate "outputs" events
    m_outputs.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, EdgeType::IS_OUTPUT_OF);
        set_dirty();
    });

    m_outputs.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, EdgeType::IS_OUTPUT_OF);
        set_dirty();
    });

    // propagate "children" events
    m_children.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, EdgeType::IS_CHILD_OF);
        set_dirty();
    });

    m_children.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, EdgeType::IS_CHILD_OF);
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

void Node::set_label(const char* _label, const char* _short_label)
{
	m_label       = _label;
	m_short_label = _short_label == nullptr ? _label : _short_label;
}

const char* Node::get_label()const
{
	return m_label.c_str();
}

void Node::add_wire(Wire* _wire)
{
	m_wires.push_back(_wire);
    m_dirty = true;
}

void Node::remove_wire(Wire* _wire)
{
	auto found = std::find(m_wires.begin(), m_wires.end(), _wire);
	if(found != m_wires.end())
		m_wires.erase(found);
    m_dirty = true;
}

std::vector<Wire*>& Node::get_wires()
{
	return m_wires;
}

int Node::get_input_wire_count()const
{
	int count = 0;
	for(auto each_wire : m_wires)
	{
		if ( each_wire->nodes.dst == this)
			count++;
	}
	return count;
}

int Node::get_output_wire_count()const
{
	int count = 0;
	for(auto each_wire : m_wires)
	{
		if ( each_wire->nodes.src == this)
			count++;
	}
	return count;
}

bool Node::eval() const
{
    // copy values (only if connection is "by copy")
    for(Member* each_member : m_props.by_id())
    {
        Member* input = each_member->get_input();

        if( input && each_member->is_connected_by(ConnectBy_Copy)
            && !each_member->is_meta_type(R::Meta_t::s_any)
            && !input->is_meta_type(R::Meta_t::s_any) )
        {
            each_member->set(input);
        }
    }

    // update
    if(has<InvokableComponent>())
    {
        return get<InvokableComponent>()->update();
    }
    return true;
}

UpdateResult Node::update()
{
	if(has<DataAccess>())
    {
        get<DataAccess>()->update();
    }

    this->m_dirty = false;

	return UpdateResult::Success;
}

GraphNode *Node::get_inner_graph() const
{
    return this->m_inner_graph;
}

void Node::get_inner_graph(GraphNode *_graph)
{
    this->m_inner_graph = _graph;
}

const IInvokable* Node::get_connected_operator(const Member *_localMember)
{
    assert(m_props.has(_localMember));

    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(m_wires.cbegin(), m_wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->members.dst == _localMember;
    });

    /*
     * If found, we try to get the ComputeXXXXXOperator from it's source
     */
    if (found != m_wires.end() )
    {
        auto node = (*found)->nodes.src->as<Node>();
        InvokableComponent* compute_component = node->get<InvokableComponent>();
        if ( compute_component )
        {
            if (compute_component->get_signature()->is_operator() )
            {
                return compute_component->get_function();
            }
        }
    }

    return nullptr;
}

bool Node::has_wire_connected_to(const Member *_localMember)
{
    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(m_wires.cbegin(), m_wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->members.dst == _localMember;
    });

    return found != m_wires.end();
}

void Node::set_parent(Node *_node)
{
    NODABLE_ASSERT(_node != nullptr || this->m_parent != nullptr);
    this->m_parent = _node;
    set_dirty();
}

void Node::set_parent_graph(GraphNode *_parentGraph)
{
    NODABLE_ASSERT(this->m_parent_graph == nullptr); // TODO: implement parentGraph switch
    this->m_parent_graph = _parentGraph;
}

const char* Node::get_short_label() const
{
    return m_short_label.c_str();
}

Node::~Node()
{
    if ( !m_components.empty())
        delete_components();
}

size_t Node::get_component_count() const
{
    return m_components.size();
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
