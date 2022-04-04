#pragma once

#include <memory>

#include "R_Type.h"
#include "R_Qualifier.h"
#include "R_Templates.h"

namespace Nodable
{
    namespace R
    {
        class MetaType;
        typedef std::shared_ptr<MetaType> MetaType_ptr;
        typedef std::shared_ptr<const MetaType> MetaType_const_ptr;

        class MetaType
        {
        public:
            MetaType(
                    const char* _name,
                    Type _type,
                    Qualifier _qualifier = Qualifier::None ) noexcept
            : m_name(_name)
            , m_type(_type)
            , m_qualifier(_qualifier)
            {}

            const char*               get_name() const { return m_name; };
            std::string               get_fullname() const;
            Type                      get_type() const { return m_type; }
            Qualifier                 get_qualifier() const { return m_qualifier; }
            bool                      has_qualifier(Qualifier _other_qualifier) const;
            void                      add_qualifier(Qualifier _other_qualifier);
            bool                      is_exactly(MetaType_const_ptr _other)const;
            template<class T> bool    is_exactly() const { return is_exactly(reflect_type<T>::make_type()); }
            template<class T> bool    is_not() const { return !is_exactly(reflect_type<T>::make_type()); }
            bool                      is_ptr() const;

            static bool               is_ptr(MetaType_const_ptr);
            static bool               is_ref(MetaType_const_ptr);
            static MetaType_ptr       add_ref(MetaType_ptr);
            static MetaType_ptr       add_ptr(MetaType_ptr);
            static bool               is_implicitly_convertible(MetaType_const_ptr _left, MetaType_const_ptr _right);
            static MetaType_const_ptr make_ptr(MetaType_const_ptr _type);
            static MetaType_const_ptr make_ref(MetaType_const_ptr _type);

            static MetaType_const_ptr s_unknown;

        protected:
            const char*     m_name;
            Type            m_type;
            Qualifier       m_qualifier;
        };
    }
}