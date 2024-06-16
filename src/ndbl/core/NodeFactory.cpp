#include "NodeFactory.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "tools/core/reflection/reflection"

#include "ForLoopNode.h"
#include "IfNode.h"
#include "InvokableComponent.h"
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

void destroy(Node* node)
{
#if !TOOLS_POOL_ENABLE
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

VariableNode* NodeFactory::create_variable(const type *_type, const std::string& _name, Scope* _scope) const
{
    // create
    auto node = create<VariableNode>(_type, _name.c_str());
    node->init();
    if( _scope )
    {
        _scope->add_variable( node );
    }
    else
    {
        LOG_WARNING("HeadlessNodeFactory", "Variable %s has been created without defining its scope.\n", _name.c_str())
    }

    m_post_process(node);

    return node;
}

Node* NodeFactory::create_abstract_func(const func_type* _signature, bool _is_operator) const
{
    Node* node = create_abstract_func_no_postprocess(_signature, _is_operator);
    add_invokable_component(node, _signature, nullptr, _is_operator );
    m_post_process(node);
    return node;
}

Node* NodeFactory::create_abstract_func_no_postprocess(const tools::func_type *_func_type, bool _is_operator) const
{
    Node* node = create<Node>();
    node->init();
    node->add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY );
    node->add_slot(SlotFlag_OUTPUT, 1); // Can be connected to an InstructionNode

    if( _is_operator )
    {
        node->set_name(_func_type->get_identifier().c_str());
    }
    else
    {
        std::string id = _func_type->get_identifier();
        std::string label       = id + "()";
        std::string short_label = id.substr(0, 2) + "..()"; // ------- improve, not great.
        node->set_name(label.c_str());
    }

    // Create a result/value
    auto return_prop_id = node->props.add(_func_type->get_return_type(), VALUE_PROPERTY );
    node->add_slot( SlotFlag_OUTPUT, Slot::MAX_CAPACITY, return_prop_id);

    // Create arguments
    auto args = _func_type->get_args();

    if( _is_operator ) // rename arguments
    {
        size_t count = _func_type->get_arg_count();

        EXPECT( count != 0 , "An operator cannot have zero argument" );
        EXPECT( count < 3  , "An operator cannot have more than 2 arguments" );

        switch ( count )
        {
            case 1:
            {
                auto left_prop_id = node->props.add(args[0].m_type, LEFT_VALUE_PROPERTY );
                node->add_slot( SlotFlag_INPUT, 1, left_prop_id);
                break;
            }

            case 2:
            {
                auto left_prop_id = node->props.add( args[0].m_type, LEFT_VALUE_PROPERTY );
                auto right_prop_id = node->props.add( args[1].m_type, RIGHT_VALUE_PROPERTY );
                node->add_slot( SlotFlag_INPUT, 1, left_prop_id );
                node->add_slot( SlotFlag_INPUT, 1 , right_prop_id);
                break;
            }

            default: /* no warning */ ;
        }
    }
    else
    {
        for (auto& arg : args)
        {
            auto each_prop_id = node->props.add(arg.m_type, arg.m_name.c_str() );
            node->add_slot( SlotFlag_INPUT, 1, each_prop_id);
        }
    }

    return node;
}

Node* NodeFactory::create_func(const IInvokable* _function, bool _is_operator) const
{
    // Create an abstract function node
    const func_type* type = _function->get_type();
    Node* node = create_abstract_func_no_postprocess(type, _is_operator);
    add_invokable_component(node, type, _function, _is_operator);
    m_post_process(node);
    return node;
}

void NodeFactory::add_invokable_component(Node* _node, const func_type* _func_type, const IInvokable*_invokable, bool _is_operator) const
{
    // Create an InvokableComponent with the function.
    auto* component = create<InvokableComponent>(_func_type, _is_operator, _invokable);
    _node->add_component(component);

    // Bind result property
    Slot* result_slot = _node->find_slot_by_property_name( VALUE_PROPERTY, SlotFlag_OUTPUT );
    component->bind_result( result_slot );

    // Link arguments
    auto args = _func_type->get_args();
    for (u8_t index = 0; index < (u8_t)args.size(); index++)
    {
        Slot& arg_slot = _node->get_nth_slot( index, SlotFlag_INPUT );
        if ( args[index].m_by_reference )
        {
            arg_slot.get_property()->flag_as_reference();  // to handle by reference function args
        }
        component->bind_arg(index, &arg_slot );
    }
}

Node* NodeFactory::create_scope() const
{
    Node* node = create<Node>();
    node->init();
    node->set_name("{} Scope");

    node->add_slot( SlotFlag_CHILD, Slot::MAX_CAPACITY );
    node->add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY );

    auto* scope = create<Scope>();
    node->add_component(scope);
    m_post_process(node);

    return node;
}

IfNode* NodeFactory::create_cond_struct() const
{
    auto* node = create<IfNode>();
    node->init();
    node->set_name("If");
    node->add_component(create<Scope>());
    node->add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
    m_post_process(node);

    return node;
}

ForLoopNode* NodeFactory::create_for_loop() const
{
    auto node = create<ForLoopNode>();
    node->init();
    node->set_name("For");
    node->add_component(create<Scope>());
    node->add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
    m_post_process(node);

    return node;
}

WhileLoopNode* NodeFactory::create_while_loop() const
{
    auto node = create<WhileLoopNode>();
    node->init();
    node->set_name("While");
    node->add_component(create<Scope>());
    node->add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
    m_post_process(node);

    return node;
}

Node* NodeFactory::create_program() const
{
    Node* node = create<Node>();
    node->init();
    node->set_name(ICON_FA_FILE_CODE " Program");
    node->add_slot( SlotFlag_CHILD, Slot::MAX_CAPACITY );
    node->add_component(create<Scope>() );
    m_post_process(node);
    return node;
}

Node* NodeFactory::create_node() const
{
    Node* node = create<Node>();
    node->init();
    node->add_slot( SlotFlag_PREV, Slot::MAX_CAPACITY);
    m_post_process(node);
    return node;
}

LiteralNode* NodeFactory::create_literal(const type *_type) const
{
    auto node = create<LiteralNode>(_type);
    node->init();
    node->set_name("Literal");
    m_post_process(node);
    return node;
}

void NodeFactory::override_post_process_fct( NodeFactory::PostProcessFct f)
{
    EXPECT( m_post_process_is_overrided == false, "Cannot override post process function more than once." );
    m_post_process_is_overrided = true;
    m_post_process              = f;
}

NodeFactory* ndbl::get_node_factory()
{
    return g_node_factory;
}

NodeFactory* ndbl::init_node_factory()
{
    ASSERT(g_node_factory == nullptr)
    g_node_factory = new NodeFactory();
    return g_node_factory;
}

void ndbl::shutdown_node_factory()
{
    ASSERT(g_node_factory != nullptr)
    delete g_node_factory;
    g_node_factory = nullptr;
}

void NodeFactory::destroy_node(Node* node) const
{
    destroy(node);
}
