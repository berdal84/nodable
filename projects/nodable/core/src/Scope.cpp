#include "ndbl/core/Scope.h"

#include <cstring>
#include <algorithm> // for std::find_if
#include "fw/core/log.h"

#include <ndbl/core/ConditionalStructNode.h>
#include <ndbl/core/ForLoopNode.h>
#include <ndbl/core/InstructionNode.h>
#include <ndbl/core/VariableNode.h>
#include <ndbl/core/IScope.h>

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<Scope>("Scope")
        .extends<IScope>()
        .extends<Node>();
}

Scope::Scope()
    : Component()
{
}

VariableNode* Scope::find_variable(const std::string &_name)
{
    VariableNode* result = nullptr;

    /*
     * Try first to find in this scope
     */
    auto has_name = [_name](const VariableNode* _variable ) -> bool
    {
        return _variable->get_name() == _name;
    };

    auto it = std::find_if(m_variables.begin(), m_variables.end(), has_name);
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
            Scope* parent_scope = owner_parent->get_component<Scope>();
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
        LOG_ERROR("Scope", "Unable to add variable '%s', already exists in the same scope.\n", _variableNode->get_name())
    }
    else if ( _variableNode->get_scope() )
    {
        LOG_ERROR("Scope", "Unable to add variable '%s', already declared in another scope. Remove it first.\n", _variableNode->get_name())
    }
    else
    {
        LOG_VERBOSE("Scope", "Add variable '%s' to the scope\n", _variableNode->get_name())
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
            if (InstructionNode* instr = each_child->as<InstructionNode>())
            {
                if (owner_children.back() == instr )
                {
                    _out.push_back(instr);
                }
            }
            else if ( Scope* scope = each_child->get_component<Scope>() )
            {
                scope->get_last_instructions_rec(_out);
            }
        }
    }
}

void Scope::remove_variable(VariableNode *_variable)
{
    FW_ASSERT(_variable)
    FW_ASSERT(_variable->get_scope() == this)
    auto found = std::find( m_variables.begin(), m_variables.end(), _variable);
    FW_ASSERT(*found)
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
