#pragma once

#include <vector>
#include <set>
#include "Token.h"
#include "tools/core/Signals.h"
#include "tools/core/Optional.h"
#include "NodeComponent.h"

namespace ndbl
{
    // forward decl.
    class Node;
    class VariableNode;
    class ScopeView;

    typedef int ScopeFlags;
    enum ScopeFlags_
    {
        ScopeFlags_NONE              = 0,
        ScopeFlags_RECURSE           = 1 << 0,
        ScopeFlags_ALLOW_CHANGE      = 1 << 1, // will automatically remove from old scope when added
        ScopeFlags_IF_SAME_NODE      = 1 << 2,
        ScopeFlags_NO_PUSH_BACK      = 1 << 3,
        ScopeFlags_CLEAR_WITH_PARENT = 1 << 4,
        ScopeFlags_INCLUDE_SELF      = 1 << 5,
    };

    class Scope : public NodeComponent
    {
    public:
        Token token_begin = {Token_t::ignore};
        Token token_end   = {Token_t::ignore};

        SIGNAL(on_reset_parent, Scope*);
        SIGNAL(on_add, Node*);
        SIGNAL(on_remove, Node*);
        SIGNAL(on_change);
        SIGNAL(on_clear);

        const char*                    name() const { return m_name.c_str(); };
        void                           set_name(const std::string& name) { m_name = name; };
        Scope*                         parent() { return m_parent; }
        const Scope*                   parent() const { return m_parent; }
        void                           reset_parent(Scope*, ScopeFlags flags = ScopeFlags_NONE);
        std::vector<Node*>             leaves();
        void                           push_back(Node* node) { push_back_ex(node, ScopeFlags_ALLOW_CHANGE | ScopeFlags_RECURSE ); }
        void                           remove(Node* node) { return remove_ex(node, ScopeFlags_ALLOW_CHANGE | ScopeFlags_RECURSE ); }
        void                           clear();
        bool                           empty() const { return m_child_node.empty(); }
        VariableNode*                  find_var(const std::string& identifier) { return find_var_ex( identifier, ScopeFlags_RECURSE ); } ;
        ScopeView*                     view() const { return m_view; }
        void                           set_view(ScopeView* view) { m_view = view; }
        const std::set<VariableNode*>& vars()const { return m_var; };
        std::vector<Node*>&            child_node() { return  m_child_node; }
        const std::vector<Node*>&      child_node() const { return m_child_node; }
        std::vector<Scope*>&           child_scope() { return m_child_scope; }
        const std::vector<Scope*>&     child_scope() const { return m_child_scope; }
        Scope*                         child_scope_at(size_t pos) { return m_child_scope.at(pos); };
        tools::Optional<Node*>         first_node() const { return m_child_node.empty() ? nullptr : *m_child_node.begin(); };
        Node*                          last_node() const { return m_child_node.empty() ? nullptr : *m_child_node.rbegin(); };
        bool                           is_orphan() const { return m_parent == nullptr; }
        static bool                    is_internal(const Scope*);
        static Scope*                  lowest_common_ancestor(const std::vector<Scope*>& scopes);
        static Scope*                  lowest_common_ancestor(Scope* s1, Scope* s2);
        static std::set<Scope*>&       get_descendent(std::set<Scope*>& out, Scope* scope, ScopeFlags flags = ScopeFlags_INCLUDE_SELF) { return get_descendent_ex(out, scope, -1, flags); }
        static std::set<Scope*>&       get_descendent_ex(std::set<Scope*>& out, Scope* scope, size_t level_max = -1, ScopeFlags = ScopeFlags_INCLUDE_SELF);
        size_t                         depth() const { return m_parent && get_owner() ? m_parent->depth() + 1 : 0; }; // TODO: precompute and store

    private:
        bool                           empty_ex(ScopeFlags) const;
        VariableNode*                  find_var_ex(const std::string& identifier, ScopeFlags);
        void                           push_back_ex(Node*, ScopeFlags);
        void                           remove_ex(Node*, ScopeFlags);
        std::vector<Node*>&            leaves_ex(std::vector<Node*>& out);

        ScopeView*                     m_view = nullptr;
        std::set<VariableNode*>        m_var;
        std::vector<Node*>             m_child_node;
        std::vector<Scope*>            m_child_scope;
        Scope*                         m_parent = nullptr;
        std::string                    m_name;

    };
}
