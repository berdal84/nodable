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
    template<typename Type = void>
    class PoolID
    {
        friend class Pool;
    public:
        using id_t = u32_t;
        static PoolID<Type> null;

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
        IPoolVector(void* _data_ptr, size_t _elem_size)
        : m_vector_ptr( _data_ptr )
        , m_elem_size(_elem_size)
        {}

        virtual ~IPoolVector() {};
        virtual std::type_index type_index() const = 0;
        virtual const char*     type_name() const = 0;
        virtual void*  at(size_t _pos ) = 0;
        virtual const void* at(size_t _pos ) const = 0;
        virtual size_t size() const = 0;
        virtual void   pop_back() = 0;
        virtual void   swap(size_t, size_t) = 0;
        virtual u32_t  poolid_at( size_t _pos ) const = 0;

        template<class T, typename ...Args>
        inline T& emplace_back(Args ...args)
        { return get<T>()->template emplace_back<>( args... ); }

        template<class T>
        inline T& emplace_back()
        { return get<T>()->template emplace_back<>(); }

        template<class T>
        inline std::vector<T>* get()
        { return (std::vector<T>*)m_vector_ptr; }

        template<class T>
        inline const std::vector<T>* get() const
        { return (const std::vector<T>*)m_vector_ptr; }
    protected:
        void*  m_vector_ptr;
        size_t m_elem_size;
    };


    /** Templated implementation of IPoolVector
     * Provide a way to deal with typed vector while respecting the IPoolVector interface */
    template<typename T>
    class TPoolVector : public IPoolVector
    {
    public:
        TPoolVector(size_t _capacity = 0)
            : IPoolVector(&m_vector, sizeof(T))
            , m_vector()
            , m_type_index{typeid(T)}
        {
            m_vector.reserve( _capacity );
        }

        ~TPoolVector()
        {};

        TPoolVector(const TPoolVector&) = delete;
        TPoolVector& operator=(const TPoolVector&) = delete;
        TPoolVector(TPoolVector&&) = delete;
        TPoolVector& operator=(TPoolVector&&) = delete;

        void* at( size_t _pos ) override
        { return &m_vector.at( _pos ); };

        const void* at( size_t _pos ) const override
        { return  &m_vector.at( _pos ); };

        void swap( size_t _a, size_t _b ) override
        { std::swap( m_vector.at( _a ), m_vector.at( _b ) ); };

        void pop_back() override
        { m_vector.pop_back(); };

        size_t size() const override
        { return m_vector.size(); };

        std::type_index type_index() const override
        { return m_type_index; }

        const char* type_name() const override
        { return  m_type_index.name(); }

        u32_t poolid_at(size_t _pos) const override
        { return (u32_t)m_vector[_pos].poolid(); }

    private:
        std::vector<T> m_vector;
        std::type_index m_type_index;
    };

    /**
     * When a new instance is created, a new record is added.
     * It allow to find the location of the object in memory and its type.
     */
    struct Record
    {
        IPoolVector* vector{ nullptr};
        size_t       pos{invalid_id<size_t>}; // Zero-based position of the data in the vector.
        u32_t        next_id{invalid_id<u32_t>}; // id to the next Record, if pos is invalid it points to the next free id.
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
        static Pool* init(size_t _capacity = 0, bool _reuse_ids = true);
        static void  shutdown();
        inline static Pool* get_pool()
        {
#ifdef NDBL_DEBUG
            FW_EXPECT(s_current_pool != nullptr, "No pool. Did you called Pool::init() ?")
#endif
            return s_current_pool;
        }
    private:
        Pool(size_t _capacity, bool _reuse_ids);
        ~Pool();
        /** for now, lets allow a single Pool at a time (see statics) */
        Pool(const Pool&) = delete;
        Pool(Pool&&) = delete;
        Pool& operator=(const Pool&) = delete;
        Pool& operator=(Pool&&) = delete;
    public:

        template<typename T>
        inline IPoolVector* init_for();

        template<typename T>
        inline T* get(u32_t id)
        {
            static_assert__is_pool_registrable<T>();
            if ( id >= m_record_by_id.size() )
            {
                return nullptr;
            }
            const auto [vector, pos, _] = m_record_by_id[id];
            return static_cast<T*>( vector->at( pos ) );
        }

        template<typename T>
        inline T* get(PoolID<T> _id)
        { return get<T>(_id.id); }

        template<typename T>
        inline std::vector<T*> get(const std::vector<PoolID<T>>& ids);

        template<typename T>
        inline std::vector<T>& get_all();

        template<typename T, typename ...Args>
        inline PoolID<T> create(Args... args);

        template<typename T>
        inline PoolID<T> create();

        template<typename T>
        inline void destroy(T* ptr);

        template<typename T>
        inline void destroy(PoolID<T> _id );

        template<typename ContainerT>
        inline void destroy_all(const ContainerT& ids);

    private:

        template<typename T>
        inline PoolID<T> make_record(T* data, IPoolVector * vec, size_t pos );

        template<typename T>
        inline IPoolVector *get_pool_vector();

        u32_t generate_id()
        {
            if( m_reuse_ids && m_first_free_id != invalid_id<u32_t> )
            {
                u32_t id = m_first_free_id;
                m_first_free_id = m_record_by_id[id].next_id; // update linked-list
                return id;
            }
            return m_record_by_id.size();
        }

        bool   m_reuse_ids;
        size_t m_initial_capacity;
        u32_t  m_first_free_id; // Linked-list of free ids
        std::vector<Record> m_record_by_id;
        std::unordered_map<std::type_index, IPoolVector*> m_pool_vector_by_type;
    private:
        static Pool* s_current_pool;
    };

    template<typename Type>
    inline Type* PoolID<Type>::get() const // Return a pointer to the data from the Pool having an id == this->id
    { return Pool::get_pool()->get<Type>( id.m_value ); }

    template<typename T>
    inline std::vector<T*> Pool::get(const std::vector<PoolID<T>>& ids)
    {
        static_assert__is_pool_registrable<T>();
        std::vector<T*> result(ids.size());
        for(size_t i = 0; i < ids.size(); ++i )
        {
            const auto    record_id = (u32_t)ids[i];
            const Record& record    = m_record_by_id[record_id];

            result[i] = (T*)record.vector->at(record.pos);
        }
        return std::move(result);
    }

    template<typename T>
    inline std::vector<T>& Pool::get_all()
    { return *get_pool_vector<T>()->template get<T>(); }

    template<typename T, typename ...Args>
    inline PoolID<T> Pool::create(Args... args)
    {
        static_assert__is_pool_registrable<T>();
        auto*  vec   = get_pool_vector<T>();
        size_t index = vec->size();
        T*     data  = &vec->template emplace_back<T>(args...);
        PoolID<T> id = make_record(data, vec, index );
        return id;
    }

    template<typename T>
    inline PoolID<T> Pool::create()
    {
        static_assert__is_pool_registrable<T>();
        IPoolVector* pool_vector = get_pool_vector<T>();
        T* data = &pool_vector->template emplace_back<T>();
        PoolID<T> id = make_record(data, pool_vector, pool_vector->size()-1 );
        return id;
    }

    template<typename T>
    inline IPoolVector* Pool::init_for()
    {
        static_assert__is_pool_registrable<T>();
        auto type_id = std::type_index(typeid(T));
        FW_ASSERT( m_pool_vector_by_type.find(type_id) == m_pool_vector_by_type.end() );
        IPoolVector* new_pool_vector = new TPoolVector<T>( m_initial_capacity );
        m_pool_vector_by_type.emplace(type_id, new_pool_vector );
        return new_pool_vector;
    }

    template<typename T>
    inline IPoolVector * Pool::get_pool_vector()
    {
        static_assert__is_pool_registrable<T>();
        auto type_id = std::type_index(typeid(T));
        // TODO: use operator[] instead of find() and force user to call init_for<T>() manually
        auto it = m_pool_vector_by_type.find( type_id );
        if ( it == m_pool_vector_by_type.end() )
        {
            LOG_VERBOSE("Pool", "No vector found for '%s'\n", fw::type::get<T>()->get_name() );
            // Not great to do the init here, but required when a type is not handled yet
            return init_for<T>();
        }
        return it->second;
    }

    template<typename T>
    inline PoolID<T> Pool::make_record(T* data, IPoolVector * vec, size_t pos )
    {
        u32_t next_id = generate_id();
        FW_ASSERT(next_id < invalid_id<u32_t>) // Last id is reserved for "null" or "invalid"
        PoolID<T> poolid{next_id};
        data->poolid(poolid);
        bool is_new_id = next_id == m_record_by_id.size();
        if( is_new_id )
        {
            m_record_by_id.push_back({vec, pos, invalid_id<u32_t>});
        }
        else
        {
            // Otherwise, reuse the Record
            m_record_by_id[next_id].pos = pos;
            m_record_by_id[next_id].vector = vec; // type can change, so vector can.
            m_record_by_id[next_id].next_id = invalid_id<u32_t>;
        }
        return poolid;
    }

    template<typename T>
    inline void Pool::destroy(T* ptr)
    {
        static_assert__is_pool_registrable<T>();
        destroy(ptr->poolid());
    }

    template<typename T>
    inline void Pool::destroy(PoolID<T> _id )
    {
        static_assert__is_pool_registrable<T>();
        Record& record_to_delete = m_record_by_id[(u32_t)_id];
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
        // From there, the record to delete is at the vector's back.
        record_to_delete.vector->pop_back();
        record_to_delete.vector = nullptr;
        // But we keep the record in memory to reuse poolid for a new instance
        record_to_delete.pos = invalid_id<u32_t>;

        if( m_reuse_ids )
        {
            // Update the "free ids" linked-list
            record_to_delete.next_id = m_first_free_id;
            m_first_free_id = (u32_t)_id;
        }
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
