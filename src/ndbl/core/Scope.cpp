#include "Scope.h"

#include <stack>
#include "bdc/String.hpp"

#include "core/Asserts.h"
#include "tools/core/Log.h"

#include "Node.h"
#include "Graph.h"

namespace ndbl
{

using namespace tools;

void    _scope_update_backbone_cache(const Scope*);
void    _scope_update_depth_cache(const Scope*);
void    _scope_set_depth_cache_dirty(const Scope*);

void scope_init(Scope* scope)
{
}

void scope_deinit(Scope* scope)
{
    VERIFY(scope->parent == nullptr, "Remove this scope from parent first");
    VERIFY(scope->children.empty(), "Scope must be empty to shutdown, since nodes can't have a nullptr scope, Graph is responsible for it");
    scope_reset_head(scope);
    assert(scope->head == nullptr);
    assert(scope->children.empty());
    assert(scope->variables.empty());
}

void _scope_update_backbone_cache(const Scope* scope)
{
    if ( !scope->_cached_backbone_dirty )
        return;

    scope->_cached_backbone.clear();
    Node* curr_node = scope->head;
    while( curr_node != nullptr && curr_node->scope == scope )
    {
        // add current
        scope->_cached_backbone.push_back(curr_node );

        // get next
        ASSERT( curr_node->flow_out()->capacity == 1 );
        Node_Slot* out = curr_node->flow_out();
        curr_node = out->first_adjacent_node();
    }
    scope->_cached_backbone_dirty = false;
}

Node* scope_find_variable(Scope* scope, const bdc::String& identifier, Scope_Flags flags )
{
    using namespace bdc;

    // Try first to find in this scope
    for(Node* node : scope->variables)
        if ( identifier == node_get_identifier(node) )
            return node;

    // not found? => recursive call in parent ...
    if ( scope->parent && flags & Scope_Flag_RECURSE_PARENT_SCOPES )
        return scope_find_variable(scope->parent, identifier, flags);

    return nullptr;
}

void scope_append(Scope* scope, Node *node)
{
    scope->_cached_backbone_dirty = true;

    const Scope* previous_scope = node->scope;
    ASSERT(node);
    VERIFY(previous_scope == nullptr, "Node should have no scope");
    VERIFY(node != scope->node, "Can't add a node into its own internal scope" );

    // Insert
    const auto& [_, ok] = scope->children.insert(node); ASSERT(ok);

    // insert as variable?
    if (node->type == Node_Type_VARIABLE )
    {
        if (scope_find_variable( scope, node_get_identifier(node)) != nullptr )
        {
            TOOLS_LOG(tools::Verbosity_Error, "Scope", "Unable to append variable '%s', already exists in the same internal_scopeview.\n", node_get_identifier(node).c_str());
            // we do not return, graph is abstract, it just won't compile ...
        }
        else if ( node->scope )
        {
            TOOLS_LOG(tools::Verbosity_Error, "Scope", "Unable to append variable '%s', already declared in another internal_scopeview. Remove it first.\n", node_get_identifier(node).c_str());
            // we do not return, graph is abstract, it just won't compile ...
        }
        else
        {
            TOOLS_LOG(tools::Verbosity_Diagnostic, "Scope", "Add '%s' variable to the internal_scopeview\n", node_get_identifier(node).c_str() );
            scope->variables.insert(node);
        }
    }

    // Insert inputs recursively
    for ( Node* input : node->inputs() )
        if ( input->type != Node_Type_VARIABLE ) // variables must be manually added
            if (input->scope == previous_scope )
                scope_append(scope, input);

    node_reset_scope(node, scope);
}

std::vector<Node*> scope_get_leaves(Scope* scope)
{
    std::vector<Node*> result;
    scope_get_leaves_ex(result, scope);
    if ( result.empty() && scope->node != nullptr )
        result.push_back( scope->node );
    return result;
}

std::vector<Node*>& scope_get_leaves_ex(std::vector<Node*>& out, Scope* scope)
{
    Node* node = scope->head;
    while( node != nullptr )
    {
        if (node->internal_scope != nullptr )
        {
            scope_get_leaves_ex(out, node->internal_scope);
        }

        auto outputs = node->flow_outputs();
        if ( outputs.empty() )
        {
            out.push_back( node );
            node = nullptr;
        }
        else
        {
            ASSERT(outputs.size() == 1); // Should happen?
            node = outputs.front();
        }
    }

    return out;
}

void scope_remove(Scope* scope, Node *node)
{
    ASSERT( node );
    VERIFY( node->scope == scope, "node can't be inside its own Scope");

    scope->_cached_backbone_dirty = true;

    // inputs first
    for ( Node* inputnode : node->inputs() )
        if ( inputnode->scope == scope )
            if ( inputnode->type != Node_Type_VARIABLE ) // variables must be manually removed
                scope_remove(scope, inputnode);

    // erase node + side effects
    scope->children.erase( node );
    if (scope->head == node )
    {
        scope_reset_head(scope);
    }
    node_reset_scope(node, nullptr);

    if ( node->type == Node_Type_VARIABLE )
    {
        scope->variables.erase(node);
    }

    ASSERT( node->scope == nullptr);
}

bool scope_is_empty(const Scope* scope, Scope_Flags flags)
{
    return scope->children.empty();
}

std::stack<Scope*> get_path(Scope* s)
{
    std::stack<Scope*> path;
    path.push(s);
    while( path.top() != nullptr )
    {
        path.push( path.top()->parent );
    }
    return path;
}

Scope* scope_find_lowest_common_ancestor(const std::set<Scope*>& scopes)
{
    Scope* lca_scope = nullptr;
    for( Scope* curr_scope : scopes )
    {
        lca_scope = lca_scope ? scope_lowest_common_ancestor( lca_scope, curr_scope )
                              : curr_scope;
    }
    return lca_scope;
}

Scope* scope_lowest_common_ancestor(Scope* s1, Scope* s2)
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

std::set<Scope*>& scope_get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max, Scope_Flags flags)
{
    ASSERT(scope != nullptr);

    if ( flags & Scope_Flag_INCLUDE_SELF )
    {
        out.insert( scope );
    }

    if ( level_max-1 == 0 )
    {
        return out;
    }

    std::vector<Node*> backbone = scope_get_backbone(scope);
    for(Node* backbone_node : backbone )
    {
        if ( backbone_node->internal_scope != nullptr )
        {
            scope_get_descendent_ex(out, backbone_node->internal_scope, level_max - 1, Scope_Flag_INCLUDE_SELF );
        }

        for(auto* output_node: backbone_node->flow_outputs())
            if ( output_node->internal_scope != nullptr )
                scope_get_descendent_ex(out, output_node->internal_scope, level_max - 1, Scope_Flag_INCLUDE_SELF );
    }

    return out;
}

void scope_reset_parent(Scope* scope, Scope* new_parent)
{
    VERIFY(new_parent != scope->parent, "new_parent is expected to be different than the current one");
    scope->parent = new_parent;
    _scope_set_depth_cache_dirty(scope);
}

void _scope_set_depth_cache_dirty(const Scope* scope)
{
    scope->_cached_depth_dirty = true;

    // recurse
    for(Node* child : scope->children)
        if (Scope* child_scope = child->internal_scope )
            _scope_set_depth_cache_dirty(child_scope);
}

bool scope_contains(const Scope* scope, Node* node)
{
    return scope->children.contains( node );
}

void scope_reset_head(Scope* scope, Node* new_head)
{
#ifdef TOOLS_DEBUG
    VERIFY( !new_head      || new_head->scope      == scope, "Node must be from this scope");
    VERIFY( !scope->head || scope->head->scope == scope, "node as backbone head should never be removed before to reset backbone head")
#endif
    scope->head = new_head;
}

void _scope_update_depth_cache(const Scope* scope)
{
    if ( !scope->_cached_depth_dirty )
        return;

    scope->_cached_depth       = scope->parent ? scope_get_depth(scope->parent) + 1 : 0;
    scope->_cached_depth_dirty = false;
}

size_t scope_get_depth(const Scope* scope)
{
    _scope_update_depth_cache(scope); return scope->_cached_depth;
};

const std::vector<Node*>& scope_get_backbone(const Scope* scope)
{
    _scope_update_backbone_cache(scope);
    return scope->_cached_backbone;
}

std::set<Scope*>& scope_get_descendent(std::set<Scope*>& out, Scope* scope, Scope_Flags flags )
{
    return scope_get_descendent_ex(out, scope, -1, flags);
}

} // namespace ndbl
