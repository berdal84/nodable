#pragma once

#include <vector>
#include <set>
#include "tools/core/Signals.h"
#include "tools/core/Optional.h"
#include "tools/core/ComponentFor.h"
#include "ASTToken.h"
#include "ASTNode.h"

namespace ndbl
{
    // forward decl.
    class ASTNode;
    class ASTVariable;
    class ASTScopeView;

    typedef int ScopeFlags;
    enum ScopeFlags_
    {
        ScopeFlags_NONE                    = 0,
        ScopeFlags_RECURSE_PARENT_SCOPES   = 1 << 0,
        ScopeFlags_RECURSE_CHILD_PARTITION = 1 << 1,
        ScopeFlags_INCLUDE_SELF            = 1 << 2,
        ScopeFlags_PREVENT_EVENTS          = 1 << 3,
    };

    class ASTScope : public tools::ComponentFor<ASTNode>
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

        void                           init(size_t partition_count = 0);
        void                           shutdown();
        bool                           contains(ASTNode* node) const;
        ASTScope*                      parent() { return m_parent; }
        const ASTScope*                parent() const { return m_parent; }
        std::vector<ASTNode*>          leaves();
        bool                           empty(ScopeFlags = ScopeFlags_NONE) const;
        ASTVariable*                   find_variable(const std::string& identifier, ScopeFlags = ScopeFlags_RECURSE_PARENT_SCOPES);
        ASTScopeView*                  view() const { return m_view; } // TODO: remove this, use components
        void                           set_view(ASTScopeView* view) { m_view = view; } // TODO: remove this, use components
        const std::set<ASTVariable*>&  variable()const { return m_variable; };
        const std::set<ASTNode*>&      child() const { return m_child; }
        void                           reset_backbone_head(ASTNode* node = nullptr);
        bool                           has_backbone_head() const { return m_backbone_head != nullptr; }
        const std::vector<ASTNode*>&   backbone() const; // return a list of nodes from the entry_point to the end of the scope (result is cached, recomputed only when necessary)
        void                           reset_parent(ASTScope* new_parent = nullptr, ScopeFlags = ScopeFlags_NONE);
        bool                           is_partitioned() const { return !m_partition.empty(); }
        bool                           is_partition() const { return m_parent && m_parent->is_partitioned(); }
        std::vector<ASTScope*>&        partition() { return m_partition; }
        const std::vector<ASTScope*>&  partition() const { return m_partition; }
        ASTScope*                      partition_at(size_t pos) { return m_partition.at(pos); };
        bool                           is_orphan() const { return m_parent == nullptr; }
        size_t                         depth() const { return m_depth; };
        ASTNode*                       node() const { return m_entity; };
        static ASTScope*               lowest_common_ancestor(const std::vector<ASTScope*>& scopes);
        static ASTScope*               lowest_common_ancestor(ASTScope* s1, ASTScope* s2);
        static std::set<ASTScope*>&    get_descendent(std::set<ASTScope*>& out, ASTScope* scope, ScopeFlags flags = ScopeFlags_INCLUDE_SELF) { return get_descendent_ex(out, scope, -1, flags); }
        static std::set<ASTScope*>&    get_descendent_ex(std::set<ASTScope*>& out, ASTScope* scope, size_t level_max = -1, ScopeFlags = ScopeFlags_INCLUDE_SELF);
        static void                    init_scope(ASTNode* unscoped_node, ASTScope *scope);
        static void                    reset_scope(ASTNode* scoped_node);
        static void                    transfer_children_to(ASTScope* source, ASTScope* target);
        static void                    change_scope(ASTNode *node, ASTScope* desired_scope, ScopeFlags = ScopeFlags_NONE);
    private:
        void                           _append(ASTNode *, ScopeFlags = ScopeFlags_NONE);
        void                           _remove(std::vector<ASTNode*>& out_removed, ASTNode *node, ScopeFlags = ScopeFlags_NONE);
        std::vector<ASTNode*>&         _leaves_ex(std::vector<ASTNode*>& out);
        void                           _update_backbone_cache();

        ASTScopeView*                  m_view = nullptr;
        std::set<ASTNode*>             m_child;
        ASTNode*                       m_backbone_head = nullptr;
        std::vector<ASTNode*>          m_backbone_cache;
        bool                           m_backbone_cache_is_dirty = true;
        std::set<ASTVariable*>         m_variable;
        std::vector<ASTScope*>         m_partition;
        ASTScope*                      m_parent = nullptr;
        size_t                         m_depth = 0;
    };
}
