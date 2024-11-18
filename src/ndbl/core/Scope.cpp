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

Scope::~Scope()
{
    // clear nodes
    clear();

    // clear partitions
    for(Scope* partition : m_partition)
        partition->reset_parent();
    m_partition.clear();

    // checks
    assert(m_related.empty());
    assert(m_variable.empty());
    assert(m_child.empty());
    assert(m_partition.empty());
}

VariableNode* Scope::_find_var_ex(const std::string& _identifier, ScopeFlags flags )
{
    // Try first to find in this scope
    for(auto it = m_variable.begin(); it != m_variable.end(); it++)
        if ( (*it)->get_identifier() == _identifier )
            return *it;

    // not found? => recursive call in parent ...
    if ( m_parent && flags & ScopeFlags_RECURSE )
        return m_parent->_find_var_ex(_identifier, flags);

    return nullptr;
}

void Scope::_push_back_ex(Node *node, ScopeFlags flags)
{
    ASSERT(node);
    _push_back(node, flags);

    if ( ( flags & ScopeFlags_RECURSE) && !node->has_internal_scope() )
    {
        ScopeFlags flags_non_primary = flags & ~ScopeFlags_AS_PRIMARY_CHILD;
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                _push_back_ex(input, flags_non_primary );

        for ( Node* next : node->flow_outputs() )
            _push_back_ex(next, flags);
    }
}

std::vector<Node*> Scope::leaves()
{
    std::vector<Node*> result;
    _leaves_ex(result);
    if ( result.empty() )
        result.push_back( node() );
    return result;
}

std::vector<Node*>& Scope::_leaves_ex(std::vector<Node*>& out)
{
    if ( !m_partition.empty() )
    {
        for( Scope* partition : m_partition )
            partition->_leaves_ex(out);
        return out; // when a scope as sub scopes, we do not consider its node as potential leaves since they are usually secondary nodes, so we return early.
    }

    if ( !m_child.empty() )
    {
        for( Node* _child_node : m_child )
            if (_child_node->has_internal_scope() )
                _child_node->internal_scope()->_leaves_ex(out); // Recursive call on nested scopes

        if ( !m_child.back()->has_internal_scope() )
            out.push_back(m_child.back() );
    }

    return out;
}

void Scope::_erase_ex(Node* node, ScopeFlags flags)
{
    ASSERT(node != nullptr);
    _erase(node);

    // recursive call(s)
    if ( ( flags & ScopeFlags_RECURSE) && !node->has_internal_scope() )
    {
        for ( Node* input : node->inputs() )
            if ( !Utils::is_instruction(input) )
                _erase_ex(input, flags & ~ScopeFlags_AS_PRIMARY_CHILD);

        for ( Node* next : node->flow_outputs() )
            _erase_ex(next, flags);
    }
}

void Scope::clear()
{
    while( !m_related.empty() )
    {
        _erase_ex(*m_related.begin(), ScopeFlags_NONE);
    }

    ASSERT(m_related.size() == 0);
    ASSERT(m_variable.size() == 0);
    ASSERT(m_child.size() == 0);

    on_clear.emit();
}

bool Scope::empty_ex(ScopeFlags flags) const
{
    bool is_empty = empty();

    if ( flags & ScopeFlags_RECURSE )
        for( const Scope* partition : m_partition )
            is_empty &= partition->empty_ex( flags );

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

    for ( Scope* partition : scope->m_partition )
    {
        out.insert( partition );
        get_descendent_ex(out, partition, level_max - 1 );
    }

    for( Node* _child_node : scope->m_child )
    {
        if ( Scope* internal_scope = _child_node->internal_scope() )
        {
            out.insert( internal_scope );
            get_descendent_ex(out, internal_scope, level_max - 1, ScopeFlags_INCLUDE_SELF );
        }
    }

    return out;
}

void Scope::change_node_scope( Node* node, Scope* desired_scope )
{
    if ( Scope* current_scope = node->scope() )
    {
        current_scope->erase(node);
    }

    if ( desired_scope )
    {
        desired_scope->push_back(node);
    }
}

bool Scope::_erase(Node *node)
{
    VERIFY( node->scope() == this, "Node does not have this as scope");
    if ( m_related.erase(node ) == 0 )
        return false;

    auto it = std::find(m_child.begin(), m_child.end(), node );
    if (it != m_child.end() )
    {
        m_child.erase(it );

        if ( node->type() == NodeType_VARIABLE )
            m_variable.erase(static_cast<VariableNode*>(node) );
    }

    node->m_parent_scope = nullptr;
    if ( node->has_internal_scope() )
        node->internal_scope()->reset_parent();
    on_change.emit();
    on_remove.emit(node);
    return true;
}

void Scope::_push_back(Node* node, ScopeFlags flags)
{
    VERIFY(node->scope() == nullptr, "Node should have no scope");

    if (flags & ScopeFlags_AS_PRIMARY_CHILD )
    {
        m_child.push_back(node );

        if ( node->type() == NodeType_VARIABLE )
        {
            auto variable_node = static_cast<VariableNode*>( node );
            if (find_variable_recursively(variable_node->get_identifier()) != nullptr )
            {
                LOG_ERROR("Scope", "Unable to _push_back variable '%s', already exists in the same internal_scope.\n", variable_node->get_identifier().c_str());
                // we do not return, graph is abstract, it just won't compile ...
            }
            else if (variable_node->scope() )
            {
                LOG_ERROR("Scope", "Unable to _push_back variable '%s', already declared in another internal_scope. Remove it first.\n", variable_node->get_identifier().c_str());
                // we do not return, graph is abstract, it just won't compile ...
            }
            else
            {
                LOG_VERBOSE("Scope", "Add '%s' variable to the internal_scope\n", variable_node->get_identifier().c_str() );
                m_variable.insert(variable_node);
            }
        }
    }
    node->m_parent_scope = this;
    if ( node->has_internal_scope() )
        node->internal_scope()->reset_parent( this );
    m_related.insert(node );
    on_change.emit();
    on_add.emit(node);
}

void Scope::reset_parent( Scope* new_parent )
{
    m_parent = new_parent;
    m_depth  = new_parent ? new_parent->m_depth + 1 : 0;
    on_reset_parent.emit(new_parent );
}

void Scope::init_partition(std::vector<Scope*>& partition )
{
    VERIFY(partition.size() > 0, "Count must be greater than 0");
    VERIFY(m_partition.empty(), "Scope::init_partition() must be called once");

    m_partition = partition;

    for(size_t i = 0; i <= partition.size(); ++i )
    {
        std::string partition_name = m_name
                                   + " (part " + std::to_string(i) + "/" + std::to_string(partition.size()) + ")";
        m_partition[i]->reset_name(partition_name);
        m_partition[i]->reset_parent(this);
    }
}
