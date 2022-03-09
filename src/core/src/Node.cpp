#include <algorithm>    // for std::find
#include <utility>
#include <nodable/Nodable.h>
#include <nodable/Node.h>
#include <nodable/Log.h> // for LOG_DEBUG(...)
#include <nodable/Wire.h>
#include <nodable/DataAccess.h>
#include <nodable/InvokableComponent.h>

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
    Member* this_member = m_props.add(THIS_MEMBER_NAME, Visibility::Always, R::get_meta_type<Node *>(), Way::Way_Out);
    this_member->set( this );

    // propagate "inputs" events
    m_inputs.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, Relation_t::IS_INPUT_OF);
        set_dirty();
    });

    m_inputs.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, Relation_t::IS_INPUT_OF);
        set_dirty();
    });


    // propagate "outputs" events
    m_outputs.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, Relation_t::IS_OUTPUT_OF);
        set_dirty();
    });

    m_outputs.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, Relation_t::IS_OUTPUT_OF);
        set_dirty();
    });

    // propagate "children" events
    m_children.m_on_added.connect( [this](Node* _node){
        m_on_relation_added.emit(_node, Relation_t::IS_CHILD_OF);
        set_dirty();
    });

    m_children.m_on_removed.connect( [this](Node* _node){
        m_on_relation_removed.emit(_node, Relation_t::IS_CHILD_OF);
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

void Node::set_label(const char* _label)
{
	this->m_label = _label;
}

void Node::set_label(std::string _label)
{
	this->m_label = _label;
}

const char* Node::get_label()const
{
	return this->m_label.c_str();
}

void Node::add_wire(Wire* _wire)
{
	m_wires.push_back(_wire);
}

void Node::remove_wire(Wire* _wire)
{
	auto found = std::find(m_wires.begin(), m_wires.end(), _wire);
	if(found != m_wires.end())
		m_wires.erase(found);
}

std::vector<Wire*>& Node::get_wires()
{
	return m_wires;
}

int Node::get_input_wire_count()const
{
	int count = 0;
	for(auto w : m_wires)
	{
		if (w->getTarget()->get_owner() == this)
			count++;
	}
	return count;
}

int Node::get_output_wire_count()const
{
	int count = 0;
	for(auto w : m_wires)
	{
		if (w->getSource()->get_owner() == this)
			count++;
	}
	return count;
}

bool Node::eval() const
{
    // copy values from input connections
    for(auto& eachNameToMemberPair : m_props.get_members())
    {
        Member* eachMember = eachNameToMemberPair.second;
        if(eachMember->get_input() && eachMember->is_connected_by(ConnectBy_Copy) )
        {
            // transfer value from member's input to member
            eachMember->set_meta_type(eachMember->get_input()->get_meta_type()); // dynamic type, TODO: show a warning ?
            eachMember->set(eachMember->get_input());
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

const InvokableOperator* Node::get_connected_operator(const Member *_localMember)
{
    assert(m_props.has(_localMember));

    const InvokableOperator* result{};

    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(m_wires.cbegin(), m_wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
    });

    /*
     * If found, we try to get the ComputeXXXXXOperator from it's source
     */
    if (found != m_wires.end() )
    {
        auto node = (*found)->getSource()->get_owner()->as<Node>();
        InvokableComponent* compute_component = node->get<InvokableComponent>();
        if ( compute_component )
        {
            const IInvokable* function = compute_component->get_invokable();
            if (function->get_invokable_type() == IInvokable::Type::Operator )
            {
                result = reinterpret_cast<const InvokableOperator*>( function );
            }
        }
    }

    return result;

}

bool Node::has_wire_connected_to(const Member *_localMember)
{
    /*
     * Find a wire connected to _member
     */
    auto found = std::find_if(m_wires.cbegin(), m_wires.cend(), [_localMember](const Wire* wire)->bool {
        return wire->getTarget() == _localMember;
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

void Node::set_short_label(const char *_label) {
    this->m_short_label = _label;
}

const char* Node::get_short_label() const {
    return this->m_short_label.c_str();
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

