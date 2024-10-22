#include "Scope.h"

#include <cstring>
#include <algorithm> // for std::find_if

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"

#include "ForLoopNode.h"
#include "IScope.h"
#include "IfNode.h"
#include "VariableNode.h"
#include "Utils.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<Scope>("Scope");
}

Scope::Scope()
    : NodeComponent()
{
}

VariableNode* Scope::find_variable(const std::string &_identifier)
{
    // Try first to find in this scope
    for(auto it = m_variables.begin(); it < m_variables.end(); it++)
        if ( (*it)->get_identifier() == _identifier )
            return *it;

    // In case not found, find recursively
    Node* parent = m_owner->parent();
    ASSERT(parent != m_owner);
    if ( !parent )
        return nullptr;
    auto* scope = parent->get_component<Scope>();
    if ( !scope )
        return nullptr;
    ASSERT(scope != this);
    return scope->find_variable( _identifier );
}

void Scope::add_variable(VariableNode* _variable)
{
    if (find_variable(_variable->get_identifier()) != nullptr )
    {
        LOG_ERROR("Scope", "Unable to add variable '%s', already exists in the same scope.\n", _variable->get_identifier().c_str())
    }
    else if ( _variable->get_scope() )
    {
        LOG_ERROR("Scope", "Unable to add variable '%s', already declared in another scope. Remove it first.\n", _variable->get_identifier().c_str())
    }
    else
    {
        LOG_VERBOSE("Scope", "Add variable '%s' to the scope\n", _variable->get_identifier().c_str() )
        m_variables.push_back(_variable);
        _variable->reset_scope(this);
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
        ASSERT(children[i]);
        if ( Scope* scope = children[i]->get_component<Scope>() )
        {
            scope->get_last_instructions_rec(_out); // Recursive call on nested scopes
        }
        else if ( i == children.size() - 1 && Utils::is_instruction( children[i] ) ) // last instruction ?
        {
            _out.push_back( children[i] ); // Append the last instruction to the result
        }
    }

    return _out;
}

void Scope::remove_variable(VariableNode* _variable)
{
    ASSERT(_variable != nullptr);
    ASSERT(_variable->get_scope() == this);
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
