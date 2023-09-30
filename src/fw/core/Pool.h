#pragma once
#include "TIdentifier.h"
#include "assertions.h"
#include <algorithm>
#include <cstdlib>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

#define AGNOSTIC_VECTOR_LAMBDA_BASED 1

#if NDBL_DEBUG
#include "reflection/type.h"
#endif

#define POOL_REGISTRABLE( Class ) \
protected: \
    template<typename T> using ID     = ::fw::ID<T>; \
    template<typename T> using PoolID = ::fw::PoolID<T>; \
    PoolID<Class> m_id; \
public: \
    using is_pool_registrable = std::true_type; \
    PoolID<Class> poolid() const { return m_id; }; \
    void poolid(PoolID<Class> _id) { m_id = _id; }

template<typename T>
void static_assert__is_pool_registrable()
{
    static_assert( std::is_same_v<typename T::is_pool_registrable, std::true_type>, "This type is not pool registrable, use POOL_REGISTRABLE macros." );
}

namespace fw
{
    /**
     * PoolID is the equivalent of a pointer, but it points to a Pool item.
     * It can be used like regular pointer, except that de-referencing cost a bit more.
     *
     * Example:
     *
     * PoolID<Type> id = ...;  // we assume we have a valid id
     * id->do_something(); // de-reference from a pool id (cost a bit more) + call a method on the pointer.
     *
     * Type* ptr = id.get(); // de-referencing once before doing lots of calls is faster.
     * ptr->do_this();
     * ...
     * ptr->do_that();
     */
    template<typename Type>
    class PoolID
    {
        friend class Pool;
    public:
        using id_t = u32_t;
        static PoolID<Type>   null;
        static constexpr id_t invalid_id = ID32<Type>::invalid_id;

        ID32<Type> id;

        PoolID() = default;

        explicit PoolID(id_t _id)
        : id(_id)
        {}

        template<typename OtherT>
        PoolID(const PoolID<OtherT>& other)
        : id((id_t)other)
        {}

        explicit PoolID(const ID<Type>& _id)
        : id(_id)
        {}

        Type* get() const; // Return a pointer to the data from the Pool having an id == this->id

        void reset()
        { id.reset(); }

        inline explicit operator bool () const
        { return (bool)id; }

        inline explicit operator id_t () const
        { return id; }

        inline PoolID<Type>& operator=(const PoolID<Type> other)
        { id = other.id; return *this; }

        inline bool operator==(const PoolID<Type>& other) const
        { return id == other.id; }

        inline bool operator!=(const PoolID<Type>& other) const
        { return id != other.id; }

        inline Type* operator -> ()
        { return get(); }

        inline Type* operator -> () const
        { return get(); }

        inline Type& operator *  ()
        { return *get(); }

        inline Type& operator *  () const
        { return *get(); }
    };

    template<typename T>
    PoolID<T> PoolID<T>::null{};

    class IPoolVector
    {
    public:
        IPoolVector() {}
        virtual ~IPoolVector() {};

        virtual std::type_index type_index() const = 0;
        virtual const char*     type_name() const = 0;
        virtual void*  vector() = 0;
        virtual void*  at(size_t index) = 0;
        virtual size_t size() const = 0;
        virtual void   pop_back() = 0;
        virtual void   swap(size_t, size_t) = 0;
        virtual size_t elem_size() const = 0;
        virtual void*  data() = 0;
        virtual u32_t  poolid_at( size_t _pos ) const = 0;

        template<class T, typename ...Args>
        inline T& emplace_back(Args ...args)
        { return get<T>()->template emplace_back<>( args... ); }

        template<class T>
        inline T& emplace_back()
        { return get<T>()->template emplace_back<>(); }

        template<class T>
        inline std::vector<T>* get()
        {
            FW_ASSERT(std::type_index(typeid(T)) == this->type_index() );
            return (std::vector<T>*)this->vector();
        }
    };


    /** Templated implementation of IPoolVector
     * Provide a way to deal with typed vector while respecting the IPoolVector interface */
    template<typename T>
    class TPoolVector : public IPoolVector
    {
    public:
        TPoolVector(size_t _reserved = 0)
        { m_vector.reserve( _reserved ); }

        ~TPoolVector() = default;

        void swap(size_t a, size_t b) override
        { std::swap( m_vector[a], m_vector[b] ); };

        void pop_back() override
        { m_vector.pop_back(); };

        void* at(size_t index) override
        { return &m_vector.at(index); };

        size_t size() const override
        { return m_vector.size(); };

        std::type_index type_index() const override
        { return m_type_index; }

        const char* type_name() const override
        { return  m_type_index.name(); }

        void* vector() override
        { return &m_vector; }

        size_t elem_size() const override
        { return sizeof(T); }

        void* data() override
        { return m_vector.data(); }

        u32_t poolid_at(size_t _pos) const override
        { return (u32_t)m_vector[_pos].poolid(); }

    private:
        std::type_index m_type_index{typeid(T)};
        std::vector<T>  m_vector;
    };

    /**
     * When a new instance is created, a new record is added.
     * It allow to find the location of the object in memory and its type.
     */
    struct Record
    {
        IPoolVector * vector;
        size_t pos;  // Zero-based position of the data in the vector.
    };

    /**
     * Handle memory for a given set of types.
     *
     * Types must be initialized using init_for<MyType>() prior to use.
     * Then, a new instance can be created with create<MyType>()
     *
     * Pointers are provided for convenience but should not be stored.
     * Users should use id to store references to a given object.
     * The pool guarantee the id is still valid even if the memory layout changed.
     */
    class Pool
    {
    public:
        static Pool* init(size_t reserved_size = 0);
        static void  shutdown();
        inline static Pool* get_pool()
        {
#ifdef NDBL_DEBUG
            FW_EXPECT(s_current_pool != nullptr, "No pool. Did you called Pool::init() ?")
#endif
            return s_current_pool;
        }
    private: /** for now, lets allow a single Pool at a time (see statics) */
        Pool(size_t reserved_size);
        ~Pool();
    public:

        template<typename T>
        inline void init_for();

        template<typename T>
        inline T* get(u32_t id)
        {
            static_assert__is_pool_registrable<T>();
            if( id == PoolID<T>::invalid_id )
            {
                return nullptr;
            }
            FW_ASSERT( id < m_record_by_id.size() );
            Record& record = m_record_by_id[id];
            const u32_t pos = record.pos;
            if( pos >= record.vector->size() )
            {
                return nullptr;
            }
            return static_cast<T*>( record.vector->at(pos));
        }

        template<typename T>
        inline T* get(PoolID<T> _id)
        { return get<T>(_id.id); }

        template<typename T>
        inline std::vector<T*> get(std::vector<PoolID<T>> ids);

        template<typename T>
        inline std::vector<T>& get_all();

        template<typename T, typename ...Args>
        inline PoolID<T> create(Args... args);

        template<typename T>
        inline PoolID<T> create();

        template<typename T>
        inline void destroy(T* ptr);

        template<typename T>
        inline void destroy(PoolID<T> pool_id);

        template<typename ContainerT>
        inline void destroy_all(const ContainerT& ids);

    private:

        template<typename T>
        inline PoolID<T> make_record(T* data, IPoolVector * vec, size_t pos );

        template<typename T>
        inline IPoolVector * get_agnostic_vector();

        size_t m_reserved_size;
        u32_t   m_next_id;
        std::vector<Record> m_record_by_id;
        std::unordered_map<std::type_index, IPoolVector *> m_vector_by_type;
    private:
        static Pool* s_current_pool;
    };

    template<typename Type>
    Type* PoolID<Type>::get() const // Return a pointer to the data from the Pool having an id == this->id
    { return Pool::get_pool()->get<Type>( id.m_value ); }

    template<typename T>
    inline std::vector<T*> Pool::get(std::vector<PoolID<T>> ids)
    {
        static_assert__is_pool_registrable<T>();
        std::vector<T*> result;
        result.reserve(ids.size());
        for(auto each_id : ids )
        {
            T* ptr = get<T>(each_id);
            FW_ASSERT(ptr != nullptr);
            result.push_back( ptr );
        }
        return result;
    }

    template<typename T>
    inline std::vector<T>& Pool::get_all()
    { return *get_agnostic_vector<T>()->template get<T>(); }

    template<typename T, typename ...Args>
    inline PoolID<T> Pool::create(Args... args)
    {
        static_assert__is_pool_registrable<T>();
        LOG_VERBOSE("Pool", "Create '%s' (with args) ...\n", fw::type::get<T>()->get_name() );
        auto*  vec   = get_agnostic_vector<T>();
        size_t index = vec->size();
        T*     data  = &vec->template emplace_back<T>(args...);
        PoolID<T> id = make_record(data, vec, index );
        LOG_VERBOSE("Pool", "Create '%s' (with args) OK\n", fw::type::get<T>()->get_name() );
        LOG_FLUSH();
        return id;
    }

    template<typename T>
    inline PoolID<T> Pool::create()
    {
        static_assert__is_pool_registrable<T>();
        IPoolVector * vector  = get_agnostic_vector<T>();
        LOG_VERBOSE("Pool", "Create '%s' ...\n", fw::type::get<T>()->get_name() );
        T* data = &vector->template emplace_back<T>();
        PoolID<T> id = make_record(data, vector, vector->size()-1 );
        LOG_VERBOSE("Pool", "Create '%s' OK\n", fw::type::get<T>()->get_name() );
        LOG_FLUSH();
        return id;
    }

    template<typename T>
    inline void Pool::init_for()
    {
        static_assert__is_pool_registrable<T>();
        auto type_id = std::type_index(typeid(T));
        FW_ASSERT(m_vector_by_type.find(type_id) == m_vector_by_type.end() );
        IPoolVector * agnostic_vector = new TPoolVector<T>(m_reserved_size);
        m_vector_by_type.insert({type_id, agnostic_vector});
        LOG_VERBOSE("Pool", "Init for '%s' OK\n", fw::type::get<T>()->get_name() );
        LOG_FLUSH()
    }

    template<typename T>
    inline IPoolVector * Pool::get_agnostic_vector()
    {
        static_assert__is_pool_registrable<T>();
        auto type_id = std::type_index(typeid(T));
        // TODO: use operator[] instead of find() and force user to call init_for<T>() manually
        auto it = m_vector_by_type.find(type_id);
        if (it == m_vector_by_type.end())
        {
            LOG_VERBOSE("Pool", "No vector found for '%s'\n", fw::type::get<T>()->get_name() );
            // Not great to do the init here, but required when a type is not handled yet
            init_for<T>();
            return get_agnostic_vector<T>();
        }
        return it->second;
    }

    template<typename T>
    inline PoolID<T> Pool::make_record(T* data, IPoolVector * vec, size_t pos )
    {
        size_t next_id = m_record_by_id.size();
        FW_ASSERT(next_id < ~u32_t{0}) // Last id is reserved for "null" or "invalid"
        PoolID<T> poolid{(u32_t)next_id};
        data->poolid(poolid);
        m_record_by_id.push_back({ vec, pos });
        LOG_VERBOSE("Pool", "New record with id %zu (type: %s, index: %zu) ...\n", (u32_t)data->poolid(), fw::type::get<T>()->get_name(), pos );
        return poolid;
    }

    template<typename T>
    inline void Pool::destroy(T* ptr)
    {
        static_assert__is_pool_registrable<T>();
        destroy(ptr->poolid());
    }

    template<typename T>
    inline void Pool::destroy(PoolID<T> pool_id)
    {
        static_assert__is_pool_registrable<T>();
        Record& record_to_delete = m_record_by_id[(u32_t)pool_id];
        size_t  last_pos         = record_to_delete.vector->size() - 1;
        size_t  pos_to_delete    = record_to_delete.pos;

        // Preserve contiguous memory by swapping the record_to_delete to delete and the last.
        if( pos_to_delete != last_pos)
        {
            // swap with the back, and update back's pool id
            size_t last_poolid = record_to_delete.vector->poolid_at( last_pos );
            record_to_delete.vector->swap( record_to_delete.pos, last_pos );
            m_record_by_id[last_poolid].pos = record_to_delete.pos;
        }
        // Delete instance and record_to_delete
        record_to_delete.vector->pop_back();
        record_to_delete.pos = PoolID<T>::invalid_id;

        LOG_VERBOSE("Pool", "Destroyed record with id %zu (type: %s, pos: %zu) ...\n", (u32_t)pool_id, fw::type::get<T>()->get_name(), pos_to_delete);
    }

    template<typename ContainerT>
    inline void Pool::destroy_all(const ContainerT& ids)
    {
        for(auto each_id : ids )
        {
            destroy( each_id );
        }
    }
} // namespace fw
