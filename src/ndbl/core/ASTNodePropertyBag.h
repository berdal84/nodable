#pragma once

#include <set>
#include <memory>                 // std::shared_ptr

#include "tools/core/reflection/Type.h"
#include "tools/core/types.h"

#include "constants.h"
#include "ASTNodeProperty.h"

namespace ndbl
{
    // forward declarations
    class ASTNode;

    /**
     * @brief The Properties class is a Property* container for a given Node.
     * This class uses several indexes (by address, name, insertion order).
     */
	class ASTNodePropertyBag
    {
    public:
        static constexpr size_t THIS_ID = 0; // id of the "this" Property. A "this" Property points to its owner's ID<Node>
        typedef std::vector<ASTNodeProperty*>::iterator       iterator;
        typedef std::vector<ASTNodeProperty*>::const_iterator const_iterator;

        ~ASTNodePropertyBag();
        void             reset_owner(ASTNode* owner) { m_owner = owner; }
        iterator         begin() { return m_properties.begin(); }
        iterator         end() { return m_properties.end(); }
        const_iterator   begin() const { return m_properties.begin(); }
        const_iterator   end() const { return m_properties.end(); }
        size_t           size() const;
        bool             has(const char*) const;
        ASTNodeProperty*        at(size_t pos );
        const ASTNodeProperty*  at(size_t pos ) const;
        ASTNodeProperty*        find_by_name(const char* _name);
        const ASTNodeProperty*  find_by_name(const char* _name) const;
        ASTNodeProperty*        find_first(PropertyFlags, const tools::TypeDescriptor* );
        const ASTNodeProperty*  find_first(PropertyFlags, const tools::TypeDescriptor* ) const;
        ASTNodeProperty*        find_id_from_name(const char*) const;
        ASTNodeProperty*        get_this();
        const ASTNodeProperty*  get_this() const;
        ASTNodeProperty*        add(const tools::TypeDescriptor* _type, const char *_name, PropertyFlags = PropertyFlag_NONE );
        ASTNodeProperty*        add(ASTNodeProperty* property);

        template<typename T>
        ASTNodeProperty* add(const char* _name, PropertyFlags _flags = PropertyFlag_NONE )
        { return add(tools::type::get<T>(), _name, _flags); }

    private:
        const ASTNodeProperty* _find_first(PropertyFlags _flags, const tools::TypeDescriptor *_type) const;
        ASTNode*                   m_owner;
        std::vector<ASTNodeProperty*>  m_properties;
        std::map<std::string, ASTNodeProperty*> m_properties_by_name;
    };
}