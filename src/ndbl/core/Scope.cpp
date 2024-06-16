#include "Scope.h"

#include <cstring>
#include <algorithm> // for std::find_if

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"

#include "ForLoopNode.h"
#include "IScope.h"
#include "IfNode.h"
#include "VariableNode.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<Scope>("Scope");
}

Scope::Scope()
    : NodeComponent()
{
}

VariableNode* Scope::find_variable(const std::string &_name)
{
    /*
     * Try first to find in this scope
     */
    auto it = std::find_if(
            m_variables.begin(),
            m_variables.end(),
            [&_name](VariableNode* _variable ) -> bool
    {
        return _variable->name == _name;
    });

    if (it != m_variables.end())
    {
        return *it;
    }

    /*
     * In case not found, find recursively
     */
    Node* parent = m_owner->find_parent();
    ASSERT(parent != m_owner);
    if ( !parent )
    {
        return {};
    }
    auto* scope = parent->get_component<Scope>();
    if ( !scope )
    {
        return {};
    }
    ASSERT(scope != this);
    return scope->find_variable( _name );
}

void Scope::add_variable(VariableNode* _variableNode)
{
    if ( find_variable(_variableNode->name) != nullptr )
    {
        LOG_ERROR("Scope", "Unable to add variable '%s', already exists in the same scope.\n", _variableNode->name.c_str())
    }
    else if ( _variableNode->get_scope() )
    {
        LOG_ERROR("Scope", "Unable to add variable '%s', already declared in another scope. Remove it first.\n", _variableNode->name.c_str())
    }
    else
    {
        LOG_VERBOSE("Scope", "Add variable '%s' to the scope\n", _variableNode->name.c_str() )
        m_variables.push_back(_variableNode);
        _variableNode->reset_scope(this);
    }
}

std::vector<Node*>& Scope::get_last_instructions_rec( std::vector<Node*>& _out)
{
    std::vector<Node*> children = m_owner->children();

    if ( children.empty() )
    {
        return _out;
    }

    // Recursive call for nested scopes
    for( size_t i = 0; i < children.size(); ++i )
    {
        ASSERT(children[i])
        if ( Scope* scope = children[i]->get_component<Scope>() )
        {
            scope->get_last_instructions_rec(_out); // Recursive call on nested scopes
        }
        else if ( i == children.size() - 1 && children[i]->is_instruction() ) // last instruction ?
        {
            _out.push_back( children[i] ); // Append the last instruction to the result
        }
    }

    return _out;
}

void Scope::remove_variable(VariableNode* _variable)
{
    ASSERT(_variable != nullptr)
    ASSERT(_variable->get_scope() == this)
    auto found = std::find(m_variables.begin(), m_variables.end(), _variable);
    ASSERT(found != m_variables.end());
    _variable->reset_scope();
    m_variables.erase( found );
}

size_t Scope::remove_all_variables()
{
    size_t count = m_variables.size();
    for(VariableNode* each_variable : m_variables)
    {
        each_variable->reset_scope();
    }
    m_variables.clear();
    return count;
}

std::vector<Node*> Scope::get_last_instructions_rec()
{
    std::vector<Node*> result;
    get_last_instructions_rec(result);
    return result;
}
