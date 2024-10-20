#include "ASTNodeFactory.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <utility>

#include "tools/core/reflection/reflection"

#include "ASTForLoopNode.h"
#include "ASTConditionalNode.h"
#include "ASTFunctionNode.h"
#include "ASTLiteralNode.h"
#include "ASTUtils.h"
#include "ASTScope.h"
#include "ASTVariableNode.h"
#include "ASTWhileLoopNode.h"
#include "ASTComponentFactory.h"

using namespace ndbl;
using namespace tools;

static ASTNodeFactory* g_node_factory{nullptr };

ASTNodeFactory::ASTNodeFactory()
: m_post_process([](ASTNode* _node){})
, m_post_process_is_overrided(false)
{
    m_component_factory = init_component_factory();
}

ASTNodeFactory::~ASTNodeFactory()
{
    shutdown_component_factory( m_component_factory );
}

template<typename T, typename ...Args >
T* new_node(Args... args)
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

void ASTNodeFactory::destroy_node(ASTNode* node) const
{
#if !TOOLS_POOL_ENABLE
    for(auto each_component : node->get_components() )
        m_component_factory->destroy(each_component);
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

ASTVariableNode* ASTNodeFactory::create_variable(const TypeDescriptor* _type, const std::string& _name, ASTScope* _scope) const
{
    // create
    auto node = new_node<ASTVariableNode>();
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

ASTVariableRefNode* ASTNodeFactory::create_variable_ref() const
{
    auto node = new_node<ASTVariableRefNode>();
    node->init();
    m_post_process(node);
    return node;
}

ASTFunctionNode* ASTNodeFactory::create_function(const FunctionDescriptor* _func_type, ASTNodeType _node_type) const
{
    auto* node = new_node<ASTFunctionNode>();
    ASSERT( _node_type == ASTNodeType_OPERATOR || _node_type == ASTNodeType_FUNCTION )
    node->init(_node_type, _func_type);
    m_post_process(node);
    return node;
}

ASTNode* ASTNodeFactory::create_scope() const
{
    ASTNode* node = new_node<ASTNode>();
    node->init(ASTNodeType_BLOCK_SCOPE, "{} Scope");

    node->add_slot(node->value(), SlotFlag_PARENT, 1);
    node->add_slot(node->value(), SlotFlag_NEXT, 1);
    node->add_slot(node->value(), SlotFlag_CHILD, Slot::MAX_CAPACITY);
    node->add_slot(node->value(), SlotFlag_PREV, Slot::MAX_CAPACITY);

    node->add_component( m_component_factory->create<ASTScope>() );

    m_post_process(node);

    return node;
}

ASTConditionalNode* ASTNodeFactory::create_cond_struct() const
{
    auto* node = new_node<ASTConditionalNode>();
    node->init("If");

    m_post_process(node);

    return node;
}

ASTForLoopNode* ASTNodeFactory::create_for_loop() const
{
    auto node = new_node<ASTForLoopNode>();
    node->init("For");

    m_post_process(node);

    return node;
}

ASTWhileLoopNode* ASTNodeFactory::create_while_loop() const
{
    auto node = new_node<ASTWhileLoopNode>();
    node->init("While");

    m_post_process(node);

    return node;
}

ASTNode* ASTNodeFactory::create_program() const
{
    ASTNode* node = new_node<ASTNode>();
    node->init(ASTNodeType_BLOCK_SCOPE, ICON_FA_FILE_CODE " Program");
    node->add_slot(node->value(), SlotFlag_PARENT, 1);
    node->add_slot(node->value(), SlotFlag_NEXT, 1);
    node->add_slot(node->value(), SlotFlag_CHILD, Slot::MAX_CAPACITY);

    node->add_component( m_component_factory->create<ASTScope>() );

    m_post_process(node);
    return node;
}

ASTNode* ASTNodeFactory::create_node() const
{
    ASTNode* node = new_node<ASTNode>();
    node->init(ASTNodeType_DEFAULT, "");
    node->add_slot( node->value(), SlotFlag_PARENT, 1);
    node->add_slot( node->value(), SlotFlag_NEXT, 1);
    node->add_slot( node->value(), SlotFlag_PREV, Slot::MAX_CAPACITY);
    m_post_process(node);
    return node;
}

ASTLiteralNode* ASTNodeFactory::create_literal(const TypeDescriptor *_type) const
{
    auto node = new_node<ASTLiteralNode>();
    node->init(_type, "Literal");
    m_post_process(node);
    return node;
}

void ASTNodeFactory::override_post_process_fct(ASTNodeFactory::PostProcessFct _function)
{
    VERIFY(m_post_process_is_overrided == false, "Cannot override post process function more than once." );
    m_post_process_is_overrided = true;
    m_post_process              = std::move(_function);
}

ASTNodeFactory* ndbl::get_node_factory()
{
    return g_node_factory;
}

ASTNodeFactory* ndbl::init_node_factory()
{
    ASSERT(g_node_factory == nullptr) // singleton
    g_node_factory = new ASTNodeFactory();
    return g_node_factory;
}

void ndbl::shutdown_node_factory(ASTNodeFactory* _factor)
{
    ASSERT(g_node_factory == _factor)  // singleton
    ASSERT(g_node_factory != nullptr)
    delete g_node_factory;
    g_node_factory = nullptr;
}

