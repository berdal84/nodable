#pragma once

#include <memory>

#include "R_Type.h"
#include "R_Qualifier.h"
#include "R_Templates.h"

namespace Nodable
{
    namespace R
    {
        class Meta_t;
        typedef std::shared_ptr<Meta_t>       Meta_t_sptr;
        typedef std::shared_ptr<const Meta_t> Meta_t_csptr;

        class Meta_t
        {
        public:
            constexpr Meta_t(
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
            bool                      has_qualifier(Qualifier) const;
            void                      add_qualifier(Qualifier);
            bool                      is_exactly(Meta_t_csptr)const;
            template<class T> bool    is_exactly() const { return is_exactly(reflect_t<T>::make_type()); }
            template<class T> bool    is_not() const { return !is_exactly(reflect_t<T>::make_type()); }
            bool                      is_ptr() const;
            bool                      is_ref() const;

            static bool               is_ptr(Meta_t_csptr);
            static bool               is_ref(Meta_t_csptr);
            static Meta_t_sptr        add_ref(Meta_t_sptr);
            static Meta_t_sptr        add_ptr(Meta_t_sptr);
            static bool               is_implicitly_convertible(Meta_t_csptr _left, Meta_t_csptr _right);
            static Meta_t_csptr       make_ptr(Meta_t_csptr _type);
            static Meta_t_csptr       make_ref(Meta_t_csptr _type);

            static Meta_t_csptr s_any;

        protected:
            const char*     m_name;
            Type            m_type;
            Qualifier       m_qualifier;
        };
    }
}