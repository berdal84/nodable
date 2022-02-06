#include "nodable/Scope.h"

#include <cstring>
#include <algorithm> // for std::find_if

#include <nodable/Log.h>
#include <nodable/ConditionalStructNode.h>
#include <nodable/ForLoopNode.h>
#include <nodable/InstructionNode.h>
#include <nodable/VariableNode.h>
#include <nodable/AbstractScope.h>

using namespace Nodable;

REFLECT_DEFINE_CLASS(Scope)

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
        return strcmp(_variable->getName(), _name.c_str()) == 0;
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
    auto children = get_owner()->get_children();
    if ( children.empty() )
        return nullptr;
    return children.back();
}

void Scope::add_variable(VariableNode* _variableNode)
{
    if ( !find_variable(_variableNode->getName()) )
    {
        m_variables.push_back(_variableNode);
    }
    else
    {
        LOG_ERROR("Scope", "Unable to add variable %s, already declared.\n", _variableNode->getName())
    }
}

void Scope::get_last_instructions(std::vector<InstructionNode *> & _out)
{
    auto owner_children = get_owner()->get_children();
    if ( owner_children.empty())
        return;

    Node *last = owner_children.back();
    if (last)
    {
        if (auto* node = last->as<InstructionNode>())
        {
            _out.push_back(node);
        }
        else if ( auto scope = last->get<Scope>() )
        {
            scope->get_last_instructions(_out);
        }
    }
}
