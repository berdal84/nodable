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
{
    CONNECT(ComponentFor::on_owner_init, &ASTScope::on_init_owner, this);
}

ASTScope::~ASTScope()
{
    // checks
    assert(m_child.empty());
    assert(m_variable.empty());
    assert(m_primary_node.empty());
    assert(m_partition.empty());
}

void ASTScope::on_init_owner(ASTNode* owner)
{
    m_node = owner; // note: Emtity might not hold this component, but ASTScope should be able to function
}

ASTVariable* ASTScope::find_variable(const std::string& _identifier, ScopeFlags flags )
{
    // Try first to find in this scope
    for(auto it = m_variable.begin(); it != m_variable.end(); it++)
        if ( (*it)->get_identifier() == _identifier )
            return *it;

    // not found? => recursive call in parent ...
    if ( m_parent && flags & ScopeFlags_RECURSE_PARENT_SCOPES )
        return m_parent->find_variable(_identifier, flags);

    return nullptr;
}

bool ASTScope::_append(ASTNode *node, ScopeFlags flags)
{
    ASSERT(node);
    VERIFY(node->scope() == nullptr, "Node should have no scope");
    VERIFY(node != this->node(), "Can't add a node into its own internal scope" );

    if (flags & ScopeFlags_AS_PRIMARY_CHILD )
    {
        m_primary_node.push_back(node );

        if (node->type() == ASTNodeType_VARIABLE )
        {
            auto variable_node = reinterpret_cast<ASTVariable*>( node );
            if (find_variable(variable_node->get_identifier()) != nullptr )
            {
                LOG_ERROR("Scope", "Unable to _push_back_item variable '%s', already exists in the same internal_scope.\n", variable_node->get_identifier().c_str());
                // we do not return, graph is abstract, it just won't compile ...
            }
            else if (variable_node->scope() )
            {
                LOG_ERROR("Scope", "Unable to _push_back_item variable '%s', already declared in another internal_scope. Remove it first.\n", variable_node->get_identifier().c_str());
                // we do not return, graph is abstract, it just won't compile ...
            }
            else
            {
                LOG_VERBOSE("Scope", "Add '%s' variable to the internal_scope\n", variable_node->get_identifier().c_str() );
                m_variable.insert(variable_node);
            }
        }
    }

    const ASTScope* curr_scope = node->scope();
    const auto& [it, ok] = m_child.insert(node );

    if ( ok )
    {
        node->reset_scope(this);
        if ( node->has_internal_scope() )
            node->internal_scope()->reset_parent( this );

        for ( ASTNode* input : node->inputs() )
            if ( input->scope() == curr_scope )
                _append(input, flags & ~ScopeFlags_AS_PRIMARY_CHILD );

        if ( flags & ScopeFlags_AS_PRIMARY_CHILD )
            for ( ASTNode* next : node->flow_outputs() )
                if ( next->scope() == curr_scope )
                    _append(next, flags);

        on_change.emit();
        on_add.emit(node);
    }

    return ok;
}

std::vector<ASTNode*> ASTScope::leaves()
{
    std::vector<ASTNode*> result;
    _leaves_ex(result);
    if ( result.empty() && m_node != nullptr )
        result.push_back( m_node );
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
    ASSERT( node );
    ASSERT( node->scope() == this); // node can't be inside its own Scope

    // inputs first
    for ( ASTNode* input : node->inputs() )
        if ( input->scope() == this )
            this->_remove(removed_nodes, input, flags & ~ScopeFlags_AS_PRIMARY_CHILD );

    // if node is a primary node
    // erase all the primary node, from the last to this one
    auto it = std::find(m_primary_node.begin(), m_primary_node.end(), node );
    while( it != m_primary_node.end() )
    {
        removed_nodes.push_back( *it );
        if ( (*it)->type() == ASTNodeType_VARIABLE )
            m_variable.erase( static_cast<ASTVariable*>(*it) );
        it = m_primary_node.erase( it );
    }

    // erase node + side effects
    m_child.erase( node );
    node->reset_scope(nullptr);
    if ( node->has_internal_scope() )
        node->internal_scope()->reset_parent(); // TODO: this could be removed, internal_scope must always have the same parent
    removed_nodes.push_back(node);

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

    // pre-checks
    ASSERT( desired_scope != nullptr);
    ASSERT( current_scope != nullptr);

    std::vector<ASTNode*> removed_nodes;
    current_scope->_remove(removed_nodes, node, flags);

#ifdef TOOLS_DEBUG
    for (auto* _node : removed_nodes )
        ASSERT( _node->scope() == nullptr);
#endif

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

void ASTScope::init_partition(std::vector<ASTScope*>& partition )
{
    VERIFY(partition.size() > 0, "Count must be greater than 0");
    VERIFY(m_partition.empty(), "Scope::init_partition() must be called once");

    m_partition = partition;

    size_t i = 0;
    for(ASTScope* _scope : m_partition )
    {
        std::string partition_name = name()
                                   + " (part " + std::to_string(i++) + "/" + std::to_string(partition.size()) + ")";
        _scope->set_name(partition_name);
        _scope->reset_parent(this);
    }
}

void ASTScope::init_scope(ASTNode* node, ASTScope* scope)
{
    ASSERT(node->scope() == nullptr);
    scope->_append(node);
}

void ASTScope::change_children_scope(ASTScope* source, ASTScope* target)
{
    VERIFY(source, "a source Scope is required");
    VERIFY(target, "a target Scope is required");

    // remove primary child
    if ( !source->primary_child().empty() )
        change_scope(source->primary_child().front(), target, ScopeFlags_AS_PRIMARY_CHILD);
    ASSERT( source->primary_child().empty());

    // remove remaining child
    for (ASTNode* _child : source->child() )
        change_scope(_child, target );
    ASSERT( source->child().empty());

    // Also clean any partition
    for( ASTScope* _scope : source->partition() )
    {
        change_children_scope(_scope, target);
    }

    VERIFY(source->empty(), "should be empty after having moved all its children" );
}

void ASTScope::reset_scope(ASTNode* node)
{
    ASSERT(node->scope());
    std::vector<ASTNode*> removed_node;
    node->scope()->_remove( removed_node, node );
    ASSERT(removed_node.size() >= 1);
}

bool ASTScope::contains(ASTNode* node) const
{
    return m_child.contains( node );
}

bool ASTScope::is_primary(ASTNode* node) const
{
    return std::find(m_primary_node.begin(), m_primary_node.end(), node) != m_primary_node.end();
}
