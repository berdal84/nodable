#include "NodeFactory.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <utility>

#include "tools/core/reflection/reflection"

#include "ForLoopNode.h"
#include "IfNode.h"
#include "FunctionNode.h"
#include "LiteralNode.h"
#include "Utils.h"
#include "Scope.h"
#include "VariableNode.h"
#include "WhileLoopNode.h"

using namespace ndbl;
using namespace tools;

static NodeFactory* g_node_factory{ nullptr };

NodeFactory::NodeFactory()
: m_post_process([](Node* _node){})
, m_post_process_is_overrided(false)
{}

template<typename T, typename ...Args >
T* create(Args... args)
{
#if !TOOLS_POOL_ENABLE
    return new T(args...);
#else
    PoolManager* manager = get_pool_manager();
    ASSERT( manager != nullptr );
    Pool* pool = manager->get_pool();
    ASSERT( pool != nullptr );
    PoolID<T> node_id = pool->create<T>(args...);
    return node_id.get();
#endif
}

void NodeFactory::destroy_node(Node* node) const
{
#if !TOOLS_POOL_ENABLE
    for(auto each_component : node->get_components() )
        delete each_component;
    delete node;
#else
    PoolManager* manager = get_pool_manager();
    ASSERT( manager != nullptr )
    Pool* pool = manager->get_pool();
    ASSERT( pool != nullptr )
    pool->destroy_all( node->get_components() );
    pool->destroy( node );
#endif
}

VariableNode* NodeFactory::create_variable(const TypeDescriptor* _type, const std::string& _name) const
{
    // create
    auto node = create<VariableNode>();
    node->init(_type, _name.c_str());
    m_post_process(node);

    return node;
}

VariableRefNode* NodeFactory::create_variable_ref() const
{
    auto node = create<VariableRefNode>();
    node->init();
    m_post_process(node);
    return node;
}

FunctionNode* NodeFactory::create_function(const FunctionDescriptor& _func_type, NodeType _node_type) const
{
    auto* node = create<FunctionNode>();
    ASSERT( _node_type == NodeType_OPERATOR || _node_type == NodeType_FUNCTION );
    node->init(_node_type, _func_type);
    m_post_process(node);
    return node;
}

IfNode* NodeFactory::create_cond_struct() const
{
    auto* node = create<IfNode>();
    node->init("If");

    m_post_process(node);

    return node;
}

ForLoopNode* NodeFactory::create_for_loop() const
{
    auto node = create<ForLoopNode>();
    node->init("For");

    m_post_process(node);

    return node;
}

WhileLoopNode* NodeFactory::create_while_loop() const
{
    auto node = create<WhileLoopNode>();
    node->init("While");

    m_post_process(node);

    return node;
}

Node* NodeFactory::create_entry_point() const
{
    Node* node = create<Node>();
    node->init(NodeType_ENTRY_POINT, ICON_FA_ARROW_ALT_CIRCLE_DOWN " BEGIN");
    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->init_internal_scope();
    m_post_process(node);
    return node;
}

Node* NodeFactory::create_node() const
{
    Node* node = create<Node>();
    node->init(NodeType_DEFAULT, "");
    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_IN, Slot::MAX_CAPACITY);
    m_post_process(node);
    return node;
}

LiteralNode* NodeFactory::create_literal(const TypeDescriptor *_type) const
{
    auto node = create<LiteralNode>();
    node->init(_type, "Literal");
    m_post_process(node);
    return node;
}

void NodeFactory::override_post_process_fct( NodeFactory::PostProcessFct _function)
{
    VERIFY(m_post_process_is_overrided == false, "Cannot override post process function more than once." );
    m_post_process_is_overrided = true;
    m_post_process              = std::move(_function);
}

Node *NodeFactory::create_empty_instruction()const
{
    Node* node = create<Node>();
    node->init(NodeType_EMPTY_INSTRUCTION, ";");

    // Token will be/or not overriden as a Token_t::end_of_instruction by the parser
    node->value()->set_token({Token_t::ignore});

    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_IN , Slot::MAX_CAPACITY);
    node->add_slot(node->value(), SlotFlag_OUTPUT  , 1);

    m_post_process(node);

    return node;
}

NodeFactory* ndbl::get_node_factory()
{
    return g_node_factory;
}

NodeFactory* ndbl::init_node_factory()
{
    ASSERT(g_node_factory == nullptr); // singleton
    g_node_factory = new NodeFactory();
    return g_node_factory;
}

void ndbl::shutdown_node_factory(NodeFactory* _factor)
{
    ASSERT(g_node_factory == _factor);  // singleton
    ASSERT(g_node_factory != nullptr);
    delete g_node_factory;
    g_node_factory = nullptr;
}

