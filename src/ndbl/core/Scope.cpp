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
    for(auto it = m_var.begin(); it != m_var.end(); it++)
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

    if ( node->scope() )
    {
        if ( flags & ScopeFlags_ALLOW_CHANGE )
            node->scope()->remove( node );
        else
            ASSERT(false); // Not handled yet
    }


    if ( node->type() == NodeType_VARIABLE )
    {
        auto variable_node = static_cast<VariableNode*>( node );
        if (find_var(variable_node->get_identifier()) != nullptr )
        {
            LOG_ERROR("Scope", "Unable to add variable '%s', already exists in the same inner_scope.\n", variable_node->get_identifier().c_str());
            // we do not return, graph is abstract, it just won't compile ...
        }
        else if (variable_node->scope() )
        {
            LOG_ERROR("Scope", "Unable to add variable '%s', already declared in another inner_scope. Remove it first.\n", variable_node->get_identifier().c_str());
            // we do not return, graph is abstract, it just won't compile ...
        }
        else
        {
            LOG_VERBOSE("Scope", "Add '%s' variable to the inner_scope\n", variable_node->get_identifier().c_str() );
            m_var.insert(variable_node);
        }
    }

    // Insert the node in this scope
    node->set_scope(this);
    if ( (flags & ScopeFlags_SKIP_INSERT) == 0)
        m_child_node.push_back( node );

    // If node have an inner scope, we simply reset its parent
    if ( node->inner_scope() )
    {
        node->inner_scope()->reset_parent(this);
    }
    // otherwise we might do a recursive call
    else if ( flags & ScopeFlags_RECURSE )
    {
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                push_back_ex(input, flags | ScopeFlags_SKIP_INSERT ); // SKIP_INSERT: we don't want those nodes to be part of the main children

        for ( Node* next : node->flow_outputs() )
            push_back_ex(next, flags);
    }

    on_change.emit();
}

std::vector<Node*> Scope::leaves()
{
    std::vector<Node*> result;
    leaves_ex(result);
    if ( result.empty() )
        result.push_back(get_owner());
    return result;
}

std::vector<Node*>& Scope::leaves_ex(std::vector<Node*>& out)
{
    if ( m_child_node.empty() && m_child_scope.empty() )
    {
        return out;
    }

    // Recursive call for nested nodes
    for( Node* child : m_child_node )
    {
        if ( Scope* inner_scope = child->inner_scope() )
        {
            inner_scope->leaves_ex(out); // Recursive call on nested scopes
        }
        else if ( child == *m_child_node.rbegin() )
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

    // if it's a variable, we remove it from the vars registry
    if ( node->type() == NodeType_VARIABLE )
    {
        auto variable = static_cast<VariableNode*>(node);
        m_var.erase( variable );
    }

    // remove the node from the registry
    auto it = std::find(m_child_node.begin(), m_child_node.end(), node);
    if( it != m_child_node.end() )
        m_child_node.erase( it );

    // reset scope
    node->set_scope(nullptr);

    // if we got an inner scope, we reset it and stop there.
    if ( node->inner_scope() )
    {
        node->inner_scope()->reset_parent(nullptr );
    }
    // if not, we might do a recursive call
    else if ( flags & ScopeFlags_RECURSE )
    {
        for ( Node* input : node->inputs() )
            if ( input->scope() == this )
                remove_ex(input, flags);

        for ( Node* next : node->flow_outputs() )
            remove_ex(next, flags);
    }

    on_change.emit();
}

void Scope::clear()
{
    while( !m_child_node.empty() )
    {
        remove_ex( m_child_node.back(), ScopeFlags_ALLOW_CHANGE | ScopeFlags_RECURSE );
    }

    on_clear.emit();
}

void Scope::reset_parent(Scope *parent, ScopeFlags flags)
{
    if ( m_parent )
        m_parent->m_child_scope.erase(std::find(m_parent->m_child_scope.begin(), m_parent->m_child_scope.end(), this ));

    if ( parent )
    {
        parent->m_child_scope.push_back(this);;
        if ( flags & ScopeFlags_CLEAR_WITH_PARENT)
            CONNECT( parent->on_clear, &Scope::clear );
    }

    m_parent = parent;
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

Scope *Scope::lowest_common_ancestor(Scope* s1, Scope* s2)
{
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
