#include "Scope.h"

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
        return m_parent->find_var(_identifier );

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
    m_child_node.insert(node );

    // If node have an inner scope, we simply reset its parent
    if ( node->inner_scope() )
    {
        node->inner_scope()->reset_parent(this);
    }
    // otherwise we migh do a recursive call
    else if ( flags & ScopeFlags_RECURSE )
    {
        LOG_VERBOSE("Scope", "Push back recursively Node '%s' inputs ...\n", node->name().c_str() );
        for ( Node* input : node->inputs() )
            push_back_ex(input, flags);
        LOG_VERBOSE("Scope", OK "%s's input(s) pushed.\n", node->name().c_str() );

        LOG_VERBOSE("Scope", "Push back recursively Node '%s' flow_out ...\n", node->name().c_str() );
        for ( Node* next : node->flow_outputs() )
            push_back_ex(next, flags);
        LOG_VERBOSE("Scope", OK "%s's flow_out(s) pushed.\n", node->name().c_str() );
    }

    on_change.emit();
}

std::vector<Node*> Scope::last_instr()
{
    std::vector<Node*> result;
    last_instr_ex(result);
    return result;
}

std::vector<Node*>& Scope::last_instr_ex(std::vector<Node*>& out)
{
    if ( m_child_node.empty() )
    {
        return out;
    }

    // Recursive call for nested scopes
    for( Node* child : m_child_node )
    {
        if ( Scope* inner_scope = child->inner_scope() )
        {
            inner_scope->last_instr_ex( out ); // Recursive call on nested scopes
        }
        else if (child == *m_child_node.rbegin() && Utils::is_instruction(child ) ) // last instruction ?
        {
            out.push_back( child ); // Append the last instruction to the result
        }
    }

    return out;
}

void Scope::remove_ex(Node* node, ScopeFlags flags)
{
    ASSERT(node != nullptr);

    if ( node->scope() != this || node->scope() == nullptr )
        return;

    // if it's a variable, we remove it from the vars registry
    if ( node->type() == NodeType_VARIABLE )
    {
        auto it = std::find(m_var.begin(), m_var.end(), node);
        if (it != m_var.end() ) // might be false, see add()
            m_var.erase( it );
    }

    // remove the node from the registry
    auto it = std::find(m_child_node.begin(), m_child_node.end(), node);
    if (it != m_child_node.end() ) // might be false, see add()
        m_child_node.erase(it );

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
            remove_ex(input, flags);

        for ( Node* next : node->flow_outputs() )
            remove_ex(next, flags);
    }

    on_change.emit();
}

size_t Scope::remove_all()
{
    size_t count = m_var.size();

    while( !m_child_node.empty() )
    {
        remove_ex(*m_child_node.begin(), ScopeFlags_ALLOW_CHANGE | ScopeFlags_RECURSE );
    }

    return count;
}

void Scope::reset_parent(Scope *parent)
{
    if ( m_parent )
        m_parent->m_child_scope_cache_dirty = true;

    if ( parent )
        parent->m_child_scope_cache_dirty = true;

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

void Scope::update_child_scope_cache() const
{
    auto nonconst_this = const_cast<Scope*>(this);

    if ( m_child_scope_cache_dirty )
    {
        nonconst_this->m_child_node.clear();

        // Append any inner scope from any child node
        for( Node* node : m_child_node )
            if ( Scope* inner_scope = node->inner_scope() )
                nonconst_this->m_child_scope_cache.push_back( inner_scope );

        nonconst_this->m_child_scope_cache_dirty = false;
    }
}
