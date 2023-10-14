#include "Scope.h"

#include <cstring>
#include <algorithm> // for std::find_if

#include "fw/core/log.h"
#include "fw/core/Pool.h"

#include "ConditionalStructNode.h"
#include "ForLoopNode.h"
#include "InstructionNode.h"
#include "VariableNode.h"
#include "IScope.h"

using namespace ndbl;
using fw::PoolID;

REGISTER
{
    fw::registration::push_class<Scope>("Scope");
}

Scope::Scope()
    : Component()
{
}

PoolID<VariableNode> Scope::find_variable(const std::string &_name)
{
    /*
     * Try first to find in this scope
     */
    auto it = std::find_if(
            m_variables.begin(),
            m_variables.end(),
            [&_name](PoolID<VariableNode> _variable ) -> bool
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
    PoolID<Node> parent = m_owner->find_parent();
    FW_ASSERT(parent != m_owner);
    if ( !parent )
    {
        return {};
    }
    PoolID<Scope> scope = parent->get_component<Scope>();
    if ( !scope )
    {
        return {};
    }
    FW_ASSERT(scope != m_id);
    return scope->find_variable( _name );
}

void Scope::add_variable(PoolID<VariableNode> _variableNode)
{
    if ( find_variable(_variableNode->name).get() != nullptr )
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

void Scope::get_last_instructions_rec(std::vector<InstructionNode*>& _out)
{
    std::vector<PoolID<Node>> children = m_owner->children();

    if ( children.empty() )
    {
        return;
    }

    for ( PoolID<Node> child_node_id : children )
    {
        Node* child_node = child_node_id.get();
        if ( child_node == nullptr ) continue;

        if ( auto* each_instr = fw::cast<InstructionNode>(child_node) )
        {
            if ( children.back().get() == each_instr )
            {
                _out.push_back(each_instr);
            }
        }
        else if ( Scope* scope = child_node->get_component<Scope>().get() )
        {
            scope->get_last_instructions_rec(_out);
        }
    }
}

void Scope::remove_variable(VariableNode* _variable)
{
    FW_ASSERT(_variable != nullptr)
    FW_ASSERT(_variable->get_scope() == m_id)
    auto found = std::find(m_variables.begin(), m_variables.end(), _variable->poolid() );
    FW_ASSERT(found->get() != nullptr);
    _variable->reset_scope();
    m_variables.erase( found );
}

size_t Scope::remove_all_variables()
{
    size_t count = m_variables.size();
    for(PoolID<VariableNode> each_variable : m_variables)
    {
        each_variable->reset_scope();
    }
    m_variables.clear();
    return count;
}
std::vector<InstructionNode *> Scope::get_last_instructions_rec()
{
    std::vector<InstructionNode *> result;
    get_last_instructions_rec(result);
    return result;
}
