#include "ASTScope.h"

#include <stack>
#include <cstring>
#include <algorithm> // for std::find_if

#include "tools/core/log.h"
#include "tools/core/memory/memory.h"

#include "ASTForLoop.h"
#include "ASTIf.h"
#include "ASTVariable.h"
#include "ASTUtils.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INITIALIZER
(
    DEFINE_REFLECT(ASTScope).extends<ComponentFor<ASTNode>>();
)

ASTScope::ASTScope()
: ComponentFor<ASTNode>("ASTScope")
{
}

ASTScope::~ASTScope()
{
    assert(m_child.empty());
    assert(m_variable.empty());
    assert(m_primary_node.empty());
    assert(m_partition.empty());
}

void ASTScope::init(size_t partition_count)
{
    for( size_t i = 0; i < partition_count; ++i )
    {
        std::string _name;
        _name += name();
        _name += " (part " + std::to_string(i+1) + "/" + std::to_string(partition_count) + ")";

        auto* _scope = m_entity->components()->create<ASTScope>();
        _scope->set_name(_name);
        _scope->reset_parent(this);

        m_partition.push_back(_scope);
    }
    ASSERT(m_partition.size() == partition_count);
}

void ASTScope::shutdown()
{
    while ( !m_partition.empty() )
    {
        m_entity->components()->destroy(m_partition.back() );
        m_partition.pop_back();
    }
}

ASTVariable* ASTScope::find_variable(const std::string& _identifier, ScopeFlags flags )
{
    // Try first to find in this scope
    for(ASTVariable* node : m_variable)
        if ( node->get_identifier() == _identifier )
            return node;

    // not found? => recursive call in parent ...
    if ( m_parent && flags & ScopeFlags_RECURSE_PARENT_SCOPES )
        return m_parent->find_variable(_identifier, flags);

    return nullptr;
}

void ASTScope::_append(ASTNode *node, ScopeFlags flags)
{
    constexpr ScopeFlags allowed_flags = ScopeFlags_APPEND_AS_PRIMARY_NODE
                                       | ScopeFlags_PREVENT_EVENTS;
    ASSERT( (flags & ~allowed_flags) == 0); // Incompatible flag

    const ASTScope* previous_scope = node->scope();
    ASSERT(node);
    VERIFY(previous_scope == nullptr, "Node should have no scope");
    VERIFY(node != this->node(), "Can't add a node into its own internal scope" );

    // Insert
    const auto& [_, ok] = m_child.insert(node); ASSERT(ok);

    // insert as variable?
    if (node->type() == ASTNodeType_VARIABLE )
    {
        auto variable_node = reinterpret_cast<ASTVariable*>( node );
        if (find_variable(variable_node->get_identifier()) != nullptr )
        {
            LOG_ERROR("Scope", "Unable to append variable '%s', already exists in the same internal_scope.\n", variable_node->get_identifier().c_str());
            // we do not return, graph is abstract, it just won't compile ...
        }
        else if (variable_node->scope() )
        {
            LOG_ERROR("Scope", "Unable to append variable '%s', already declared in another internal_scope. Remove it first.\n", variable_node->get_identifier().c_str());
            // we do not return, graph is abstract, it just won't compile ...
        }
        else
        {
            LOG_VERBOSE("Scope", "Add '%s' variable to the internal_scope\n", variable_node->get_identifier().c_str() );
            m_variable.insert(variable_node);
        }
    }

    // Insert as primary?
    if (flags & ScopeFlags_APPEND_AS_PRIMARY_NODE )
    {
        m_primary_node.push_back(node);

        // recursively append following nodes
        for ( ASTNode* next : node->flow_outputs() )
            if (next->scope() == previous_scope )
                _append(next, flags);
    }

    // Insert inputs recursively
    for ( ASTNode* input : node->inputs() )
        if (input->scope() == previous_scope )
            _append(input, flags & ~ScopeFlags_APPEND_AS_PRIMARY_NODE );

    node->reset_scope(this);

    // emit event
    if ( !(flags & ScopeFlags_PREVENT_EVENTS) )
    {
        on_change.emit();
        on_add.emit(node);
    }
}

std::vector<ASTNode*> ASTScope::leaves()
{
    std::vector<ASTNode*> result;
    _leaves_ex(result);
    if ( result.empty() && node() != nullptr )
        result.push_back( node() );
    return result;
}

std::vector<ASTNode*>& ASTScope::_leaves_ex(std::vector<ASTNode*>& out)
{
    if ( !m_partition.empty() )
    {
        for( ASTScope* partition : m_partition )
            partition->_leaves_ex(out);
        return out; // when a scope as sub scopes, we do not consider its node as potential leaves since they are usually secondary nodes, so we return early.
    }

    if ( !m_primary_node.empty() )
    {
        for( ASTNode* _child_node : m_primary_node )
            if (_child_node->has_internal_scope() )
                _child_node->internal_scope()->_leaves_ex(out); // Recursive call on nested scopes

        if ( !m_primary_node.back()->has_internal_scope() )
            out.push_back(m_primary_node.back() );
    }

    return out;
}

void ASTScope::_remove(std::vector<ASTNode *> &removed_nodes, ndbl::ASTNode *node, ndbl::ScopeFlags flags)
{
    constexpr ScopeFlags allowed_flags = ScopeFlags_PREVENT_EVENTS;
    ASSERT( (flags & ~allowed_flags) == 0); // Incompatible flag
    ASSERT( node );
    ASSERT( node->scope() == this); // node can't be inside its own Scope

    // inputs first
    for ( ASTNode* input : node->inputs() )
        if ( input->scope() == this )
            this->_remove(removed_nodes, input, flags );

    // if primary, we remove everything that's next
    auto node_primary_it = std::find(m_primary_node.begin(), m_primary_node.end(), node);
    if ( node_primary_it != m_primary_node.end() )
    {
        while( node != m_primary_node.back() )
        {
            _remove( removed_nodes, m_primary_node.back(), flags);
        }
        m_primary_node.erase( node_primary_it );
    }

    // erase node + side effects
    m_child.erase( node );
    node->reset_scope(nullptr);
    removed_nodes.push_back(node);
    if ( node->type() == ASTNodeType_VARIABLE )
    {
        m_variable.erase( reinterpret_cast<ASTVariable*>(node) );
    }

    if ( (flags & ScopeFlags_PREVENT_EVENTS) == 0 )
    {
        on_change.emit();
        on_remove.emit(node);
    }

#ifdef TOOLS_DEBUG
    for (auto* _node : removed_nodes )
        ASSERT( _node->scope() == nullptr );
#endif
}

bool ASTScope::empty(ScopeFlags flags) const
{
    bool is_empty = m_child.empty();

    if (flags & ScopeFlags_RECURSE_CHILD_PARTITION )
        for( const ASTScope* partition : m_partition )
            is_empty &= partition->empty(flags);

    return is_empty;
}

std::stack<ASTScope*> get_path(ASTScope* s)
{
    std::stack<ASTScope*> path;
    path.push(s);
    while( path.top() != nullptr )
    {
        path.push( path.top()->parent() );
    }
    return path;
}

ASTScope* ASTScope::lowest_common_ancestor(const std::vector<ASTScope*>& scopes)
{
    if ( scopes.empty() )
        return nullptr;
    ASTScope* result = *scopes.begin();
    for(auto it = scopes.begin()+1; it != scopes.end(); ++it)
    {
        result = lowest_common_ancestor( result, *it);
    }
    return result;
}

ASTScope* ASTScope::lowest_common_ancestor(ASTScope* s1, ASTScope* s2)
{
    if ( s1 == s2 )
    {
        return s1;
    }

    std::stack<ASTScope*> path1 = get_path(s1);
    std::stack<ASTScope*> path2 = get_path(s2);

    ASTScope* common = nullptr;
    while( !path1.empty() && !path2.empty() && path1.top() == path2.top() )
    {
        common = path1.top();
        path1.pop();
        path2.pop();
    }

    return common;
}

std::set<ASTScope*>& ASTScope::get_descendent_ex(std::set<ASTScope*>& out, ASTScope* scope, size_t level_max, ScopeFlags flags)
{
    if ( flags & ScopeFlags_INCLUDE_SELF )
    {
        out.insert( scope );
    }

    if ( level_max-1 == 0 )
        return out;

    for ( ASTScope* partition : scope->m_partition )
    {
        out.insert( partition );
        get_descendent_ex(out, partition, level_max - 1 );
    }

    for( ASTNode* _child_node : scope->m_primary_node )
    {
        if ( ASTScope* internal_scope = _child_node->internal_scope() )
        {
            out.insert( internal_scope );
            get_descendent_ex(out, internal_scope, level_max - 1, ScopeFlags_INCLUDE_SELF );
        }
    }

    return out;
}

void ASTScope::change_scope(ASTNode* node, ASTScope* desired_scope, ScopeFlags flags )
{
    ASTScope* current_scope = node->scope();
    ASSERT( desired_scope != nullptr);
    ASSERT( current_scope != nullptr);

    ASSERT( desired_scope != current_scope ); // TODO

    std::vector<ASTNode*> removed_nodes;
    current_scope->_remove(removed_nodes, node, flags & ScopeFlags_PREVENT_EVENTS); // this is the only flag useful here
    desired_scope->_append(node, flags);

#ifdef TOOLS_DEBUG
    for (auto* _node : removed_nodes )
        ASSERT( _node->scope() == desired_scope);
#endif
}

void ASTScope::reset_parent(ASTScope* new_parent, ScopeFlags flags )
{
    m_parent = new_parent;
    m_depth  = new_parent ? new_parent->m_depth + 1 : 0;

    if ( (flags & ScopeFlags_PREVENT_EVENTS) == 0 )
        on_reset_parent.emit(new_parent );
}

void ASTScope::init_scope(ASTNode* node, ASTScope* scope)
{
    ASSERT( !node->has_flags(ASTNodeFlag_WAS_IN_A_SCOPE_ONCE) );
    scope->_append(node);
}

void ASTScope::transfer_children_to(ASTScope* source, ASTScope* target)
{
    ASSERT(source);
    ASSERT(target);

    // remove primary child
    while( !source->primary_child().empty() )
        change_scope(source->primary_child().back(), target, ScopeFlags_APPEND_AS_PRIMARY_NODE );

    // remove remaining child
    while ( !source->child().empty() )
        change_scope(*source->child().rbegin(), target );

    // Also clean any partition
    for( ASTScope* _scope : source->partition() )
    {
        transfer_children_to(_scope, target);
        ASSERT(_scope->empty());
    }

    ASSERT(source->empty());
}

void ASTScope::reset_scope(ASTNode* node)
{
    ASSERT(node->scope());
    std::vector<ASTNode*> removed_node;
    node->scope()->_remove( removed_node, node );
    ASSERT(!removed_node.empty());
}

bool ASTScope::contains(ASTNode* node) const
{
    return m_child.contains( node );
}