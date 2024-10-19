#include "NodeFactory.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <utility>

#include "tools/core/reflection/reflection"

#include "ForLoopNode.h"
#include "IfNode.h"
#include "FunctionNode.h"
#include "LiteralNode.h"
#include "NodeUtils.h"
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
    ASSERT( manager != nullptr )
    Pool* pool = manager->get_pool();
    ASSERT( pool != nullptr )
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

VariableNode* NodeFactory::create_variable(const TypeDescriptor* _type, const std::string& _name, Scope* _scope) const
{
    // create
    auto node = create<VariableNode>();
    node->init(_type, _name.c_str());
    if( _scope )
    {
        _scope->add_variable( node );
    }
    else
    {
        LOG_WARNING("HeadlessNodeFactory", "Variable %s has_flags been created without defining its scope.\n", _name.c_str())
    }

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

FunctionNode* NodeFactory::create_function(const FunctionDescriptor* _func_type, NodeType _node_type) const
{
    auto* node = create<FunctionNode>();
    ASSERT( _node_type == NodeType_OPERATOR || _node_type == NodeType_FUNCTION )
    node->init(_node_type, _func_type);
    m_post_process(node);
    return node;
}

Node* NodeFactory::create_scope() const
{
    Node* node = create<Node>();
    node->init(NodeType_BLOCK_SCOPE, "{} Scope");

    node->add_slot(node->value(), SlotFlag_PARENT, 1);
    node->add_slot(node->value(), SlotFlag_NEXT, 1);
    node->add_slot(node->value(), SlotFlag_CHILD, Slot::MAX_CAPACITY);
    node->add_slot(node->value(), SlotFlag_PREV, Slot::MAX_CAPACITY);

    auto* scope = create<Scope>();
    node->add_component(scope);
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

Node* NodeFactory::create_program() const
{
    Node* node = create<Node>();
    node->init(NodeType_BLOCK_SCOPE, ICON_FA_FILE_CODE " Program");
    node->add_slot(node->value(), SlotFlag_PARENT, 1);
    node->add_slot(node->value(), SlotFlag_NEXT, 1);
    node->add_slot(node->value(), SlotFlag_CHILD, Slot::MAX_CAPACITY);
    node->add_component(create<Scope>() );
    m_post_process(node);
    return node;
}

Node* NodeFactory::create_node() const
{
    Node* node = create<Node>();
    node->init(NodeType_DEFAULT, "");
    node->add_slot( node->value(), SlotFlag_PARENT, 1);
    node->add_slot( node->value(), SlotFlag_NEXT, 1);
    node->add_slot( node->value(), SlotFlag_PREV, Slot::MAX_CAPACITY);
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

NodeFactory* ndbl::get_node_factory()
{
    return g_node_factory;
}

NodeFactory* ndbl::init_node_factory()
{
    ASSERT(g_node_factory == nullptr) // singleton
    g_node_factory = new NodeFactory();
    return g_node_factory;
}

void ndbl::shutdown_node_factory(NodeFactory* _factor)
{
    ASSERT(g_node_factory == _factor)  // singleton
    ASSERT(g_node_factory != nullptr)
    delete g_node_factory;
    g_node_factory = nullptr;
}

