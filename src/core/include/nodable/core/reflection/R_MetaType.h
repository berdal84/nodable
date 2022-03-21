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

            const char* get_name() const { return m_name; };
            std::string get_fullname() const;
            Type get_type() const { return m_type; }
            Qualifier get_qualifier() const { return m_qualifier; }
            bool has_qualifier(Qualifier _other_qualifier) const;
            void add_qualifier(Qualifier _other_qualifier);
            bool is(const std::shared_ptr<const MetaType>& _other)const;
            template<class T>
            bool is() const { return is(reflect_type<T>::make_type()); }
            template<class T>
            bool is_not() const { return !is(reflect_type<T>::make_type()); }

            static bool   is_ptr(const std::shared_ptr<const MetaType>&);
            static bool   is_ref(const std::shared_ptr<const MetaType>&);
            static std::shared_ptr<MetaType>  add_ref(std::shared_ptr<MetaType>);
            static std::shared_ptr<MetaType>  add_ptr(std::shared_ptr<MetaType>);
            static bool   is_convertible(std::shared_ptr<const MetaType> , std::shared_ptr<const MetaType> );
            static std::shared_ptr<const MetaType>  make_ptr(const std::shared_ptr<const MetaType>& _type);
            static std::shared_ptr<const MetaType>  make_ref(const std::shared_ptr<const MetaType>& _type);

            static std::shared_ptr<MetaType> s_unknown;

        protected:
            const char*     m_name;
            Type            m_type;
            Qualifier       m_qualifier;
        };
    }
}