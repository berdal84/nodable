#include "Scope.h"

#include <stack>
#include <cstring>

#include "core/Asserts.h"
#include "tools/core/Log.h"

#include "Node.h"
#include "Graph.h"

using namespace ndbl;
using namespace tools;

Scope::Scope()
: Component<Node>("Scope")
{
    // Component::signal_init.connect<&Scope::on_init>(this);
    Component::signal_shutdown.connect<Scope, &scope_on_shutdown>(this);
    // Component::signal_name_change.connect<&Scope::_on_name_change>(this);
}

Scope::~Scope()
{
    // assert(Component::signal_init.disconnect<&Scope::on_init>(this));
    Component::signal_shutdown.disconnect();
    // assert(Component::signal_name_change.disconnect<&Scope::_on_name_change>(this));
    assert(parent == nullptr);
    assert(head == nullptr);
    assert(children.empty());
    assert(variables.empty());
    assert(partition.empty());
}

void ndbl::_scope_update_backbone_cache(const Scope* scope)
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

void ndbl::scope_on_shutdown(Scope* scope)
{
    VERIFY_(scope->parent == nullptr, "Remove this scope from parent first");

    // reset partitions (they will be shutdown individually by the Component_Bag)
    for(Scope* partition : scope->partition )
    {
        scope_reset_parent(partition);
    }
    scope->partition.clear();

    VERIFY( scope->children.empty(), "Scope must be empty to shutdown, since nodes can't have a nullptr scope, Graph is responsible for it");
    scope_reset_head(scope);
}

Node* ndbl::scope_find_variable(Scope* scope, const std::string& _identifier, Scope_Flags flags )
{
    // Try first to find in this scope
    for(Node* node : scope->variables)
        if ( node_get_identifier(node) == _identifier )
            return node;

    // not found? => recursive call in parent ...
    if ( scope->parent && flags & Scope_Flag_RECURSE_PARENT_SCOPES )
        return scope_find_variable(scope->parent, _identifier, flags);

    return nullptr;
}

void ndbl::scope_append(Scope* scope, Node *node)
{
    scope->_cached_backbone_dirty = true;

    const Scope* previous_scope = node->scope;
    ASSERT(node);
    VERIFY(previous_scope == nullptr, "Node should have no scope");
    VERIFY(node != scope->node(), "Can't add a node into its own internal scope" );

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

std::vector<Node*> ndbl::scope_get_leaves(Scope* scope)
{
    std::vector<Node*> result;
    scope_get_leaves_ex(result, scope);
    if ( result.empty() && scope->node() != nullptr )
        result.push_back( scope->node() );
    return result;
}

std::vector<Node*>& ndbl::scope_get_leaves_ex(std::vector<Node*>& out, Scope* scope)
{
    if ( !scope->partition.empty() )
    {
        for( Scope* partition : scope->partition )
            scope_get_leaves_ex(out, partition);
        return out; // when a scope as sub scopes, we do not consider its node as potential leaves since they are usually secondary nodes, so we return early.
    }

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

void ndbl::scope_remove(Scope* scope, ndbl::Node *node)
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

bool ndbl::scope_is_empty(const Scope* scope, Scope_Flags flags)
{
    bool is_empty = scope->children.empty();

    if (flags & Scope_Flag_RECURSE_CHILD_PARTITION )
        for( const Scope* partition : scope->partition )
            is_empty &= scope_is_empty(partition, flags);

    return is_empty;
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

Scope* ndbl::scope_find_lowest_common_ancestor(const std::set<Scope*>& scopes)
{
    Scope* lca_scope = nullptr;
    for( Scope* curr_scope : scopes )
    {
        lca_scope = lca_scope ? scope_lowest_common_ancestor( lca_scope, curr_scope )
                              : curr_scope;
    }
    return lca_scope;
}

Scope* ndbl::scope_lowest_common_ancestor(Scope* s1, Scope* s2)
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

std::set<Scope*>& ndbl::scope_get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max, Scope_Flags flags)
{
    if ( flags & Scope_Flag_INCLUDE_SELF )
    {
        out.insert( scope );
    }

    if ( level_max-1 == 0 )
        return out;

    for ( Scope* partition : scope->partition )
    {
        out.insert( partition );
        scope_get_descendent_ex(out, partition, level_max - 1 );
    }

    Node* node = scope->head;
    while( node != nullptr )
    {
        if ( node->internal_scope != nullptr )
        {
            out.insert( node->internal_scope );
            scope_get_descendent_ex(out, node->internal_scope, level_max - 1, Scope_Flag_INCLUDE_SELF );
        }

        auto& outputs = node->flow_outputs();
        if ( !outputs.empty() )
        {
            ASSERT(outputs.size() == 1);
            node = outputs.front();
        }
        else
        {
            node = nullptr;
        }
    }

    return out;
}

void ndbl::scope_reset_parent(Scope* scope, Scope* new_parent)
{
    VERIFY(new_parent != scope->parent, "new_parent is expected to be different than the current one");
    scope->parent = new_parent;
    _scope_set_depth_cache_dirty(scope);
}

void ndbl::_scope_set_depth_cache_dirty(const Scope* scope)
{
    scope->_cached_depth_dirty = true;

    // recurse
    for(Node* child : scope->children)
        if (Scope* child_scope = child->internal_scope )
            _scope_set_depth_cache_dirty(child_scope);
}

bool ndbl::scope_contains(const Scope* scope, Node* node)
{
    return scope->children.contains( node );
}

void ndbl::scope_reset_head(Scope* scope, Node* new_head)
{
#ifdef TOOLS_DEBUG
    VERIFY( !new_head      || new_head->scope      == scope, "Node must be from this scope");
    VERIFY( !scope->head || scope->head->scope == scope, "node as backbone head should never be removed before to reset backbone head")
#endif
    scope->head = new_head;
}

void ndbl::_scope_update_depth_cache(const Scope* scope)
{
    if ( !scope->_cached_depth_dirty )
        return;

    scope->_cached_depth       = scope->parent ? scope_get_depth(scope->parent) + 1 : 0;
    scope->_cached_depth_dirty = false;
}
