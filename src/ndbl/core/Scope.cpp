#include "Scope.h"

#include <stack>
#include <cstring>
#include <algorithm> // for std::find_if

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"

#include "ForLoopNode.h"
#include "IfNode.h"
#include "VariableNode.h"
#include "Utils.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    type::Initializer<Scope>("Scope");
}

VariableNode* Scope::find_var_ex(const std::string& _identifier, ScopeFlags flags )
{
    // Try first to find in this scope
    for(auto it = m_variable_node.begin(); it != m_variable_node.end(); it++)
        if ( (*it)->get_identifier() == _identifier )
            return *it;

    // not found? => recursive call in parent ...
    if ( m_parent && flags & ScopeFlags_RECURSE )
        return m_parent->find_var_ex(_identifier, flags );

    return nullptr;
}

void Scope::push_back_ex(Node *node, ScopeFlags flags)
{
    ASSERT(node);
    node_register_add( node, flags );

    if ( ( flags & ScopeFlags_RECURSE) && !node->has_internal_scope() )
    {
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                push_back_ex(input, flags & ~ScopeFlags_IS_PRIMARY_NODE );

        for ( Node* next : node->flow_outputs() )
            push_back_ex(next, flags);
    }

    on_change.emit();
    on_add.emit(node);
}

std::vector<Node*> Scope::leaves()
{
    std::vector<Node*> result;
    leaves_ex(result);
    if ( result.empty() )
        result.push_back( node() );
    return result;
}

std::vector<Node*>& Scope::leaves_ex(std::vector<Node*>& out)
{
    if (m_primary_node.empty() && m_child_scope.empty() )
    {
        return out;
    }

    // Recursive call for nested nodes
    for( Node* child : m_primary_node )
    {
        if (child->has_internal_scope() )
        {
            child->internal_scope()->leaves_ex(out); // Recursive call on nested scopes
        }
        else if ( child == *m_primary_node.rbegin() )
        {
            out.push_back( child ); // Append the last instruction to the result
        }
    }

    for( Scope* child : m_child_scope )
    {
        child->leaves_ex(out); // Recursive call on nested scopes
    }

    return out;
}

void Scope::remove_ex(Node* node, ScopeFlags flags)
{
    ASSERT(node != nullptr);

    node_register_remove(node, flags);

    on_change.emit();
    on_remove.emit(node);

    // recursive call(s)
    if ( ( flags & ScopeFlags_RECURSE) && !node->has_internal_scope() )
    {
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                remove_ex(input, flags & ~ScopeFlags_IS_PRIMARY_NODE );

        for ( Node* next : node->flow_outputs() )
            remove_ex(next, flags);
    }
}

void Scope::clear()
{
    while( !m_primary_node.empty() )
    {
        remove_ex( m_primary_node.back(), ScopeFlags_IS_PRIMARY_NODE );
    }

    while( !m_all_node.empty() )
    {
        remove_ex(*m_all_node.begin(), ScopeFlags_NONE );
    }

    on_clear.emit();
}

void Scope::reset_parent(Scope* new_parent, ScopeFlags flags)
{
    // clear current parent from "this"
    if ( m_parent )
    {
        auto it = std::find(m_parent->m_child_scope.begin(), m_parent->m_child_scope.end(), this );
        m_parent->m_child_scope.erase( it );
        if ( flags & ScopeFlags_CLEAR_WITH_PARENT)
            DISCONNECT(new_parent->on_clear);
    }

    // add "this" into new parent
    if ( new_parent )
    {
        new_parent->m_child_scope.push_back(this);
        if ( flags & ScopeFlags_CLEAR_WITH_PARENT)
            CONNECT(new_parent->on_clear, &Scope::clear );
    }

    m_parent = new_parent;
    on_reset_parent.emit( new_parent );
}

bool Scope::empty_ex(ScopeFlags flags) const
{
    bool result = empty();

    if ( !result )
    {
        return result;
    }
    else if ( flags & ScopeFlags_RECURSE )
    {
        for(Scope* child : child_scope() )
            if ((flags & ScopeFlags_IF_SAME_NODE) == 0 || child->m_owner == m_owner )
                result = result && child->empty_ex( flags );
    }

    return result;
}

std::stack<Scope*> get_path(Scope* s)
{
    std::stack<Scope*> path;
    path.push(s);
    while( path.top() != nullptr )
    {
        path.push( path.top()->parent() );
    }
    return path;
}

Scope* Scope::lowest_common_ancestor(const std::vector<Scope*>& scopes)
{
    if ( scopes.empty() )
        return nullptr;
    Scope* result = *scopes.begin();
    for(auto it = scopes.begin()+1; it != scopes.end(); ++it)
    {
        result = lowest_common_ancestor( result, *it);
    }
    return result;
}

Scope* Scope::lowest_common_ancestor(Scope* s1, Scope* s2)
{
    if ( s1 == s2 )
    {
        return s1;
    }

    std::stack<Scope*> path1 = get_path(s1);
    std::stack<Scope*> path2 = get_path(s2);

    Scope* common = nullptr;
    while( !path1.empty() && !path2.empty() && path1.top() == path2.top() )
    {
        common = path1.top();
        path1.pop();
        path2.pop();
    }

    return common;
}

std::set<Scope*>& Scope::get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max, ScopeFlags flags)
{
    if ( flags & ScopeFlags_INCLUDE_SELF )
    {
        out.insert( scope );
    }

    if ( level_max-1 == 0 )
        return out;

    for(Scope* child : scope->m_child_scope)
    {
        get_descendent_ex(out, child, level_max-1, ScopeFlags_INCLUDE_SELF );
    }
    return out;
}

bool Scope::is_internal(const Scope* scope)
{
    return scope->node()->has_internal_scope()
           && scope->node()->internal_scope() == scope;
}

void Scope::change_scope(Node* node, Scope* desired_scope )
{
    Scope* current_scope = node->scope();
    if ( current_scope ) current_scope->remove( node );
    if ( desired_scope ) desired_scope->push_back( node );
}

void Scope::node_register_remove(Node* node, ScopeFlags flags)
{
    VERIFY( node->scope() == this, "Node does not have this as scope");
    const int erased_count = m_all_node.erase(node );
    VERIFY( erased_count, "Unable to find node" );

    auto it = std::find(m_primary_node.begin(), m_primary_node.end(), node );
    if ( it != m_primary_node.end() )
    {
        m_primary_node.erase( it );

        if ( node->type() == NodeType_VARIABLE )
            m_variable_node.erase( static_cast<VariableNode*>(node) );
    }

    node->reset_scope();
}

void Scope::node_register_add(Node* node, ScopeFlags flags)
{
    VERIFY(node->scope() == nullptr, "Node should have no scope");

    if ( flags & ScopeFlags_IS_PRIMARY_NODE )
    {
        m_primary_node.push_back( node );

        if ( node->type() == NodeType_VARIABLE )
        {
            auto variable_node = static_cast<VariableNode*>( node );
            if (find_var(variable_node->get_identifier()) != nullptr )
            {
                LOG_ERROR("Scope", "Unable to add variable '%s', already exists in the same internal_scope.\n", variable_node->get_identifier().c_str());
                // we do not return, graph is abstract, it just won't compile ...
            }
            else if (variable_node->scope() )
            {
                LOG_ERROR("Scope", "Unable to add variable '%s', already declared in another internal_scope. Remove it first.\n", variable_node->get_identifier().c_str());
                // we do not return, graph is abstract, it just won't compile ...
            }
            else
            {
                LOG_VERBOSE("Scope", "Add '%s' variable to the internal_scope\n", variable_node->get_identifier().c_str() );
                m_variable_node.insert(variable_node);
            }
        }
    }
    m_all_node.insert(node );
    node->reset_scope(this);
}
