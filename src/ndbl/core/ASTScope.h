#pragma once

#include <vector>
#include <set>
#include "tools/core/Signals.h"
#include "tools/core/Optional.h"
#include "tools/core/Component.h"
#include "ASTToken.h"

namespace ndbl
{
    // forward decl.
    class ASTNode;
    class ASTVariable;
    class ScopeView;

    typedef int ScopeFlags;
    enum ScopeFlags_
    {
        ScopeFlags_NONE              = 0,
        ScopeFlags_RECURSE           = 1 << 0,
        ScopeFlags_AS_PRIMARY_CHILD  = 1 << 1,
        ScopeFlags_INCLUDE_SELF      = 1 << 2,
        ScopeFlags_PREVENT_EVENTS    = 1 << 3,
    };

    class ASTScope : public tools::Component
    {
    public:
        DECLARE_REFLECT_override

        SIGNAL(on_reset_parent, ASTScope*);
        SIGNAL(on_add, ASTNode*);
        SIGNAL(on_remove, ASTNode*);
        SIGNAL(on_change);
        SIGNAL(on_clear);

        ASTToken token_begin = {ASTToken_t::ignore};
        ASTToken token_end   = {ASTToken_t::ignore};

        ASTScope();
        ~ASTScope();
        void                          on_init_owner(tools::Entity* );
        ASTScope*                         parent() { return m_parent; }
        const ASTScope*                   parent() const { return m_parent; }
        std::vector<ASTNode*>             leaves();
        void                           clear();
        bool                           empty() const { return m_child.empty(); }
        bool                           empty_ex(ScopeFlags) const;
        ASTVariable*                  find_variable_recursively(const std::string& identifier) { return _find_var_ex(identifier, ScopeFlags_RECURSE); } ;
        ScopeView*                     view() const { return m_view; }
        void                           set_view(ScopeView* view) { m_view = view; }
        void                           push_back(ASTNode* node) { _push_back_ex(node, ScopeFlags_RECURSE | ScopeFlags_AS_PRIMARY_CHILD); }
        void                           erase(ASTNode* node) { return _erase_ex(node, ScopeFlags_RECURSE); }
        const std::set<ASTVariable*>& variable()const { return m_variable; };
        std::vector<ASTNode*>&            child() { return  m_child; }
        const std::vector<ASTNode*>&      child() const { return m_child; }
        ASTNode*                          first_child() const { return m_child.empty() ? nullptr : *m_child.begin(); };
        ASTNode*                          last_child() const { return m_child.empty() ? nullptr : *m_child.rbegin(); };
        void                           reset_parent(ASTScope* new_parent = nullptr, ScopeFlags = ScopeFlags_NONE);
        bool                           is_partitioned() const { return !m_partition.empty(); }
        bool                           is_partition() const { return m_parent && m_parent->is_partitioned(); }
        std::vector<ASTScope*>&           partition() { return m_partition; }
        const std::vector<ASTScope*>&     partition() const { return m_partition; }
        ASTScope*                         partition_at(size_t pos) { return m_partition.at(pos); };
        bool                           is_orphan() const { return m_parent == nullptr; }
        size_t                         depth() const { return m_depth; };
        void                           init_partition(std::vector<ASTScope*>& partition);
        ASTNode*                      node() const { return m_node; };
        static void                    change_node_scope(ASTNode *node, ASTScope *desired_scope);
        static ASTScope*                  lowest_common_ancestor(const std::vector<ASTScope*>& scopes);
        static ASTScope*                  lowest_common_ancestor(ASTScope* s1, ASTScope* s2);
        static std::set<ASTScope*>&       get_descendent(std::set<ASTScope*>& out, ASTScope* scope, ScopeFlags flags = ScopeFlags_INCLUDE_SELF) { return get_descendent_ex(out, scope, -1, flags); }
        static std::set<ASTScope*>&       get_descendent_ex(std::set<ASTScope*>& out, ASTScope* scope, size_t level_max = -1, ScopeFlags = ScopeFlags_INCLUDE_SELF);
    private:
        ASTVariable*                  _find_var_ex(const std::string& identifier, ScopeFlags);
        void                           _push_back_ex(ASTNode*, ScopeFlags);
        void                           _erase_ex(ASTNode*, ScopeFlags);
        std::vector<ASTNode*>&            _leaves_ex(std::vector<ASTNode*>& out);
        void                           _push_back(ASTNode *node, ScopeFlags flags);
        bool                           _erase(ASTNode *node);

        ASTNode*                       m_node = nullptr;
        ScopeView*                     m_view = nullptr;
        std::set<ASTNode*>                m_related;
        std::set<ASTVariable*>        m_variable;
        std::vector<ASTNode*>             m_child;
        std::vector<ASTScope*>            m_partition;
        ASTScope*                         m_parent = nullptr;
        size_t                         m_depth = 0;
    };
}
