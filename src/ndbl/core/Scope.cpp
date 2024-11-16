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

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(Scope).extends<NodeComponent>();
)

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

void Scope::child_push_back_ex(Node *node, ScopeFlags flags)
{
    ASSERT(node);
    node_register_add( node, flags );

    if ( ( flags & ScopeFlags_RECURSE) && !node->has_internal_scope() )
    {
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                child_push_back_ex(input, flags & ~ScopeFlags_AS_PRIMARY_CHILD);

        for ( Node* next : node->flow_outputs() )
            child_push_back_ex(next, flags );
    }
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
    if ( !m_sub_scope.empty() )
    {
        for( Scope* _sub_scope : m_sub_scope )
            _sub_scope->leaves_ex(out);
        return out; // when a scope as sub scopes, we do not consider its node as potential leaves since they are usually secondary nodes, so we return early.
    }

    if ( !m_child_node.empty() )
    {
        for( Node* _child_node : m_child_node )
            if (_child_node->has_internal_scope() )
                _child_node->internal_scope()->leaves_ex(out); // Recursive call on nested scopes

        if ( !m_child_node.back()->has_internal_scope() )
            out.push_back( m_child_node.back() );
    }

    return out;
}

void Scope::child_erase_ex(Node* node, ScopeFlags flags)
{
    ASSERT(node != nullptr);

    node_register_remove(node, flags);

    // recursive call(s)
    if ( ( flags & ScopeFlags_RECURSE) && !node->has_internal_scope() )
    {
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                child_erase_ex(input, flags & ~ScopeFlags_AS_PRIMARY_CHILD);

        for ( Node* next : node->flow_outputs() )
            child_erase_ex(next, flags);
    }
}

void Scope::clear()
{
    while( !m_child_node.empty() )
    {
        child_erase_ex(m_child_node.back(), ScopeFlags_AS_PRIMARY_CHILD);
    }

    while( !m_all_node.empty() )
    {
        child_erase_ex(*m_all_node.begin(), ScopeFlags_NONE);
    }

    on_clear.emit();
}

void Scope::sub_scope_push_back( ndbl::Scope* sub_scope )
{
    m_sub_scope.push_back( sub_scope );
    sub_scope->m_parent = this;
    sub_scope->on_reset_parent.emit( this );
}

bool Scope::empty_ex(ScopeFlags flags) const
{
    bool is_empty = empty();

    for( Scope* _sub_scope : m_sub_scope )
        is_empty &= _sub_scope->empty_ex(flags );

    if ( flags & ScopeFlags_RECURSE )
        for( Scope* _sub_scope : m_sub_scope )
            is_empty &= _sub_scope->empty_ex(flags );

    return is_empty;
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

    for ( Scope* _sub_scope : scope->m_sub_scope )
    {
        out.insert(_sub_scope);
        get_descendent_ex(out, _sub_scope, level_max - 1 );
    }

    for( Node* _child_node : scope->m_child_node )
    {
        if ( _child_node->has_internal_scope() )
        {
            out.insert( _child_node->internal_scope() );
            get_descendent_ex(out, _child_node->internal_scope(), level_max - 1, ScopeFlags_INCLUDE_SELF );
        }
    }

    return out;
}

void Scope::change_scope(Node* node, Scope* desired_scope )
{
    Scope* current_scope = node->scope();
    if ( current_scope ) current_scope->child_erase(node);
    if ( desired_scope ) desired_scope->child_push_back(node);
}

void Scope::node_register_remove(Node* node, ScopeFlags flags)
{
    VERIFY( node->scope() == this, "Node does not have this as scope");
    const int erased_count = m_all_node.erase(node );
    VERIFY( erased_count, "Unable to find node" );

    auto it = std::find(m_child_node.begin(), m_child_node.end(), node );
    if (it != m_child_node.end() )
    {
        m_child_node.erase(it );

        if ( node->type() == NodeType_VARIABLE )
            m_variable_node.erase( static_cast<VariableNode*>(node) );
    }

    node->reset_scope();
    on_change.emit();
    on_remove.emit(node);
}

void Scope::node_register_add(Node* node, ScopeFlags flags)
{
    VERIFY(node->scope() == nullptr, "Node should have no scope");

    if (flags & ScopeFlags_AS_PRIMARY_CHILD )
    {
        m_child_node.push_back(node );

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
    m_all_node.insert( node );
    node->reset_scope(this);
    on_change.emit();
    on_add.emit(node);
}

void Scope::reset_parent( Scope* scope )
{
    m_parent = scope;
    on_reset_parent.emit( scope );
}
