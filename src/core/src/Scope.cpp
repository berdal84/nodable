#include "nodable/core/Scope.h"

#include <cstring>
#include <algorithm> // for std::find_if

#include <nodable/core/Log.h>
#include <nodable/core/ConditionalStructNode.h>
#include <nodable/core/ForLoopNode.h>
#include <nodable/core/InstructionNode.h>
#include <nodable/core/VariableNode.h>
#include <nodable/core/IScope.h>

using namespace ndbl;

REGISTER
{
    registration::push_class<Scope>("Scope")
        .extends<IScope>()
        .extends<Node>();
}

Scope::Scope()
    : Component()
    , m_begin_scope_token(nullptr)
    , m_end_scope_token(nullptr)
{
}

void Scope::clear()
{
    m_variables.clear();
}

VariableNode* Scope::find_variable(const std::string &_name)
{
    VariableNode* result = nullptr;

    /*
     * Try first to find in this scope
     */
    auto findFunction = [_name](const VariableNode* _variable ) -> bool
    {
        return strcmp(_variable->get_name(), _name.c_str()) == 0;
    };

    auto it = std::find_if(m_variables.begin(), m_variables.end(), findFunction);
    if (it != m_variables.end()){
        result = *it;
    }

    /*
     * In case not found, find recursively
     */
    if ( result == nullptr )
    {
        Node* owner_parent = get_owner()->get_parent();

        if ( owner_parent )
        {
            auto parent_scope = owner_parent->get<Scope>();
            if ( parent_scope )
            {
                result = parent_scope->find_variable( _name );
            }
        }
    }
    return result;
}

Node* Scope::get_last_code_block()
{
    if (get_owner()->children_slots().empty() )
        return nullptr;
    return get_owner()->children_slots().back();
}

void Scope::add_variable(VariableNode* _variableNode)
{
    if ( find_variable(_variableNode->get_name()) )
    {
        LOG_ERROR("Scope", "Unable to add variable %s, already exists in the same scope.\n", _variableNode->get_name())
    }
    else if ( _variableNode->get_scope() )
    {
        LOG_ERROR("Scope", "Unable to add variable %s, already declared in another scope. Remove it first.\n", _variableNode->get_name())
    }
    else
    {
        m_variables.push_back(_variableNode);
        _variableNode->set_scope(this);
    }
}

void Scope::get_last_instructions_rec(std::vector<InstructionNode *> & _out)
{
    auto& owner_children = get_owner()->children_slots();
    if ( owner_children.empty())
        return;

    for(auto each_child : owner_children)
    {
        if (each_child)
        {
            if (auto* instr = each_child->as<InstructionNode>())
            {
                if (owner_children.back() == instr )
                {
                    _out.push_back(instr);
                }
            }
            else if ( auto scope = each_child->get<Scope>() )
            {
                scope->get_last_instructions_rec(_out);
            }
        }
    }
}

void Scope::remove_variable(VariableNode *_variable)
{
    NDBL_ASSERT(_variable)
    NDBL_ASSERT(_variable->get_scope() == this)
    auto found = std::find( m_variables.begin(), m_variables.end(), _variable);
    NDBL_ASSERT(*found)
    _variable->set_scope(nullptr);
    m_variables.erase( found );
}

size_t Scope::remove_all_variables()
{
    size_t count = m_variables.size();
    for(VariableNode* each_variable : m_variables)
    {
        each_variable->set_scope(nullptr);
    }
    m_variables.clear();
    return count;
}
