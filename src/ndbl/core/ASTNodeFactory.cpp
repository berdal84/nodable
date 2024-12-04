#include "ASTNodeFactory.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include <utility>

#include "tools/core/reflection/reflection"

#include "ASTForLoop.h"
#include "ASTIf.h"
#include "ASTFunctionCall.h"
#include "ASTLiteral.h"
#include "ASTUtils.h"
#include "ASTScope.h"
#include "ASTVariable.h"
#include "ASTWhileLoop.h"

using namespace ndbl;
using namespace tools;

static ASTNodeFactory* g_node_factory{nullptr };

ASTNodeFactory::ASTNodeFactory()
: m_post_process([](ASTNode* _node){})
, m_post_process_is_overrided(false)
{}

template<typename T, typename ...Args >
T* create(Args... args)
{
    auto* entity = new Entity();
    return entity->emplace<T>(args...);
}

ASTVariable* ASTNodeFactory::create_variable(const TypeDescriptor* _type, const std::string& _name) const
{
    // create
    auto node = create<ASTVariable>();
    node->init(_type, _name.c_str());
    m_post_process(node);

    return node;
}

ASTVariableRef* ASTNodeFactory::create_variable_ref() const
{
    auto node = create<ASTVariableRef>();
    node->init();
    m_post_process(node);
    return node;
}

ASTFunctionCall* ASTNodeFactory::create_function(const FunctionDescriptor& _func_type, ASTNodeType _node_type) const
{
    auto* node = create<ASTFunctionCall>();
    ASSERT(_node_type == ASTNodeType_OPERATOR || _node_type == ASTNodeType_FUNCTION );
    node->init(_node_type, _func_type);
    m_post_process(node);
    return node;
}

ASTIf* ASTNodeFactory::create_cond_struct() const
{
    auto* node = create<ASTIf>();
    node->init("If");

    m_post_process(node);

    return node;
}

ASTForLoop* ASTNodeFactory::create_for_loop() const
{
    auto node = create<ASTForLoop>();
    node->init("For");

    m_post_process(node);

    return node;
}

ASTWhileLoop* ASTNodeFactory::create_while_loop() const
{
    auto node = create<ASTWhileLoop>();
    node->init("While");

    m_post_process(node);

    return node;
}

ASTNode* ASTNodeFactory::create_entry_point() const
{
    ASTNode* node = create<ASTNode>();
    node->init(ASTNodeType_ENTRY_POINT, ICON_FA_ARROW_ALT_CIRCLE_DOWN " BEGIN");
    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->init_internal_scope();
    m_post_process(node);
    return node;
}

ASTNode* ASTNodeFactory::create_node() const
{
    ASTNode* node = create<ASTNode>();
    node->init(ASTNodeType_DEFAULT, "");
    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_IN, ASTNodeSlot::MAX_CAPACITY);
    m_post_process(node);
    return node;
}

ASTLiteral* ASTNodeFactory::create_literal(const TypeDescriptor *_type) const
{
    auto node = create<ASTLiteral>();
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

ASTNode *ASTNodeFactory::create_empty_instruction()const
{
    ASTNode* node = create<ASTNode>();
    node->init(ASTNodeType_EMPTY_INSTRUCTION, ";");

    // Token will be/or not overriden as a Token_t::end_of_instruction by the parser
    node->value()->set_token({ASTToken_t::ignore});

    node->add_slot(node->value(), SlotFlag_FLOW_OUT, 1);
    node->add_slot(node->value(), SlotFlag_FLOW_IN , ASTNodeSlot::MAX_CAPACITY);
    node->add_slot(node->value(), SlotFlag_OUTPUT  , 1);

    m_post_process(node);

    return node;
}

ASTNodeFactory* ndbl::get_node_factory()
{
    return g_node_factory;
}

ASTNodeFactory* ndbl::init_node_factory()
{
    ASSERT(g_node_factory == nullptr); // singleton
    g_node_factory = new ASTNodeFactory();
    return g_node_factory;
}

void ndbl::shutdown_node_factory(ASTNodeFactory* _factor)
{
    ASSERT(g_node_factory == _factor);  // singleton
    ASSERT(g_node_factory != nullptr);
    delete g_node_factory;
    g_node_factory = nullptr;
}

