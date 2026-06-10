#include "Scope.h"

#include <stack>
#include <cstring>

#include "tools/core/Log.h"

#include "Node.h"
#include "Graph.h"

using namespace ndbl;
using namespace tools;

Scope::Scope()
: Component<Node>("Scope")
{
    // Component::signal_init.connect<&Scope::on_init>(this);
    Component::signal_shutdown.connect<&Scope::_on_shutdown>(this);
    // Component::signal_name_change.connect<&Scope::_on_name_change>(this);
}

Scope::~Scope()
{
    // assert(Component::signal_init.disconnect<&Scope::on_init>(this));
    Component::signal_shutdown.disconnect();
    // assert(Component::signal_name_change.disconnect<&Scope::_on_name_change>(this));
    assert(m_parent == nullptr);
    assert(m_head == nullptr);
    assert(m_children.empty());
    assert(m_variables.empty());
    assert(m_partition.empty());
}

void Scope::_on_shutdown()
{
    ASSERT(m_parent == nullptr); // Remove this scope from parent first

    // reset partitions (they will be shutdown individually by the Component_Bag)
    for(Scope* partition : m_partition )
    {
        partition->reset_parent(nullptr);
    }
    m_partition.clear();

    VERIFY( m_children.empty(), "Scope must be empty to shutdown, since nodes can't have a nullptr scope, Graph is responsible for it");
    reset_head();
}

Node* Scope::find_variable(const std::string& _identifier, Scope_Flags flags )
{
    // Try first to find in this scope
    for(Node* node : m_variables)
        if ( node_get_identifier(node) == _identifier )
            return node;

    // not found? => recursive call in parent ...
    if ( m_parent && flags & Scope_Flag_RECURSE_PARENT_SCOPES )
        return m_parent->find_variable(_identifier, flags);

    return nullptr;
}

void Scope::append(Node *node)
{
    m_cached_backbone_dirty = true;

    const Scope* previous_scope = node->scope;
    ASSERT(node);
    VERIFY(previous_scope == nullptr, "Node should have no scope");
    VERIFY(node != this->node(), "Can't add a node into its own internal scope" );

    // Insert
    const auto& [_, ok] = m_children.insert(node); ASSERT(ok);

    // insert as variable?
    if (node->type == Node_Type_VARIABLE )
    {
        if (find_variable( node_get_identifier(node)) != nullptr )
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
            m_variables.insert(node);
        }
    }

    // Insert inputs recursively
    for ( Node* input : node->inputs() )
        if ( input->type != Node_Type_VARIABLE ) // variables must be manually added
            if (input->scope == previous_scope )
                append(input);

    node_reset_scope(node, this);
}

std::vector<Node*> Scope::leaves()
{
    std::vector<Node*> result;
    _leaves_ex(result);
    if ( result.empty() && node() != nullptr )
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

    Node* node = m_head;
    while( node != nullptr )
    {
        if (node->internal_scope != nullptr )
        {
            node->internal_scope->_leaves_ex(out);
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

void Scope::remove(ndbl::Node *node)
{
    ASSERT( node );
    ASSERT( node->scope == this); // node can't be inside its own Scope

    m_cached_backbone_dirty = true;

    // inputs first
    for ( Node* input : node->inputs() )
        if ( input->scope == this )
            if ( !input->is_variable() ) // variables must be manually removed
                remove(input);

    // erase node + side effects
    m_children.erase( node );
    if (m_head == node )
    {
        reset_head();
    }
    node_reset_scope(node, nullptr);

    if ( node->is_variable() )
    {
        m_variables.erase(node);
    }

    ASSERT( node->scope == nullptr);
}

bool Scope::empty(Scope_Flags flags) const
{
    bool is_empty = m_children.empty();

    if (flags & Scope_Flag_RECURSE_CHILD_PARTITION )
        for( const Scope* partition : m_partition )
            is_empty &= partition->empty(flags);

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

Scope* Scope::lowest_common_ancestor(const std::set<Scope*>& scopes)
{
    Scope* lca_scope = nullptr;
    for( Scope* curr_scope : scopes )
    {
        lca_scope = lca_scope ? lowest_common_ancestor( lca_scope, curr_scope )
                              : curr_scope;
    }
    return lca_scope;
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

std::set<Scope*>& Scope::get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max, Scope_Flags flags)
{
    if ( flags & Scope_Flag_INCLUDE_SELF )
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

    Node* node = scope->m_head;
    while( node != nullptr )
    {
        if ( node->internal_scope != nullptr )
        {
            out.insert( node->internal_scope );
            get_descendent_ex(out, node->internal_scope, level_max - 1, Scope_Flag_INCLUDE_SELF );
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

void Scope::reset_parent(Scope* new_parent)
{
    VERIFY(new_parent != m_parent, "new_parent is expected to be different than the current one");
    m_parent = new_parent;
    _set_depth_cache_dirty();
}

void Scope::_set_depth_cache_dirty() const
{
    m_cached_depth_dirty = true;

    // recurse
    for(Node* child : m_children)
        if (Scope* child_scope = child->internal_scope )
            child_scope->_set_depth_cache_dirty();
}

bool Scope::contains(Node* node) const
{
    return m_children.contains( node );
}

void Scope::reset_head(Node* node)
{
#ifdef TOOLS_DEBUG
    VERIFY( !node || node->scope == this, "Node must be from this scope");
    VERIFY(!m_head || m_head->scope == this, "node as backbone head should never be removed before to reset backbone head")
#endif
    m_head = node;
}

void Scope::_update_depth_cache()  const
{
    if ( !m_cached_depth_dirty )
        return;

    m_cached_depth = m_parent ? m_parent->depth() + 1 : 0;
    m_cached_depth_dirty = false;
}

void Scope::_update_backbone_cache() const
{
    if ( !m_cached_backbone_dirty )
        return;

    m_cached_backbone.clear();
    Node* curr_node = m_head;
    while( curr_node != nullptr && curr_node->scope == this )
    {
        // add current
        m_cached_backbone.push_back(curr_node );

        // get next
        ASSERT( curr_node->flow_out()->capacity == 1 );
        Node_Slot* out = curr_node->flow_out();
        curr_node = out->first_adjacent_node();
    }
    m_cached_backbone_dirty = false;
}
