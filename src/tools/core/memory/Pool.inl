#pragma once
#include "Pool.h"
#include "PoolManager.h"

namespace tools
{
    //////////////////////////// PoolID ////////////////////////////////////////////

    template<typename T>
    inline PoolID<T>::PoolID(u64_t _id)
        : id(_id)
    {}

    template<typename T>
    inline PoolID<T>::PoolID(const ID<T>& _id)
        : id(_id)
    {}

    template<typename T>
    inline void PoolID<T>::reset()
    { id.reset(); }

    template<typename T>
    inline PoolID<T>::operator bool () const
    {
        return id != ID64<T>{};
    }

    template<typename T>
    inline PoolID<T>::operator u64_t () const
    { return (u64_t)id; }

    template<typename T>
    inline PoolID<T>& PoolID<T>::operator=(const PoolID<T> other)
    { id = other.id; return *this; }

    template<typename T>
    inline bool PoolID<T>::operator==(const PoolID<T>& other) const
    { return id == other.id; }

    template<typename T>
    inline bool PoolID<T>::operator!=(const PoolID<T>& other) const
    { return id != other.id; }

    template<typename T>
    inline T* PoolID<T>::operator -> ()
    {
        ASSERT(*this) return get(); }

    template<typename T>
    inline T* PoolID<T>::operator -> () const
    {
        ASSERT(*this) return get(); }

    template<typename T>
    inline T& PoolID<T>::operator * ()
    {
        ASSERT(*this) return *get(); }

    template<typename T>
    inline T& PoolID<T>::operator * () const
    {
        ASSERT(*this) return *get(); }

    template<typename T>
    PoolID<T> PoolID<T>::null{};


    ////////////////////////// IPoolVector /////////////////////////////////////////////////////

    inline IPoolVector::IPoolVector(void* _data_ptr, size_t _elem_size, std::type_index _type_index)
        : m_vector_ptr( _data_ptr )
        , m_elem_size(_elem_size)
        , m_type_index(_type_index)
    {}

    inline std::type_index IPoolVector::type_index() const
    { return m_type_index; }

    inline const char* IPoolVector::type_name() const
    { return  m_type_index.name(); }

    inline void* IPoolVector::operator[](size_t _pos) const
    { return (void*)(get<char>().data() + m_elem_size * _pos); }

    template<class T, typename ...Args>
    inline T& IPoolVector::emplace_back(Args ...args)
    { return get<T>().template emplace_back<>( args... ); }

    template<class T>
    inline T& IPoolVector::emplace_back()
    { return get<T>().template emplace_back<>(); }

    template<class T>
    inline std::vector<T>& IPoolVector::get()
    { return *(std::vector<T>*)m_vector_ptr; }

    template<class T>
    inline const std::vector<T>& IPoolVector::get() const
    { return *(const std::vector<T>*)m_vector_ptr; }

    ////////////////////////// Pool ///////////////////////////////////////////////////////////

    inline Pool::Pool( const Config& config )
        : m_config( config )
        , m_first_free_id( invalid_id<u64_t> )
        , m_pool_vector_by_type()
        , m_record_by_id()
    {
    }

    inline Pool::~Pool()
    {
        for(const auto& [type, pool_vector]: m_pool_vector_by_type )
        {
            delete pool_vector;
        }
    }

    template<typename T>
    inline T* Pool::get(u64_t id)
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        if ( id >= m_record_by_id.size() )
        {
            return nullptr;
        }
        const auto [vector, pos, _] = m_record_by_id[id];
        if ( vector == nullptr)
        {
            return nullptr;
        }
        return (T*)((*vector)[pos]);
    }

    template<typename T>
    inline T* Pool::get(PoolID<T> _id)
    {
#if TOOLS_NO_POOL
        return (T*)(u64_t)_id;
#else
        return get<T>( (u64_t)_id );
#endif
    }

    inline u64_t Pool::generate_id()
    {
        if( m_config.reuse_ids && m_first_free_id != invalid_id<u64_t> )
        {
            u64_t id = m_first_free_id;
            m_first_free_id = m_record_by_id[id].next_id; // update linked-list
            return id;
        }
        ASSERT( m_record_by_id.size() != invalid_id<u64_t> )
        return (u64_t)m_record_by_id.size();
    }

    template<typename Type>
    inline Type* PoolID<Type>::get() const // Return a pointer to the data from the Pool having an id == this->id
    {
#ifdef TOOLS_NO_POOL
        return (Type*)(u64_t)id;
#else
        if( id ) return get_pool_manager()->get_pool()->get<Type>( id.m_value );
        return nullptr;
#endif
    }

    template<typename T>
    inline std::vector<T*> Pool::get(const std::vector<PoolID<T>>& ids)
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        std::vector<T*> result(ids.size()); // TODO: remove allocation? Add std::vector<T*>& _out ?
        for(size_t i = 0; i < ids.size(); ++i )
        {
            result[i] = get(ids[i]);
        }
        return std::move(result);
    }

    template<typename T>
    inline void Pool::get(std::vector<T*>& _out, const std::vector<PoolID<T>>& ids)
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        ASSERT( ids.size() <= _out.size() );
        for(size_t i = 0; i < ids.size(); ++i )
        {
            _out[i] = get(ids[i]);
        }
    }

    template<typename T>
    inline std::vector<T>& Pool::get_all()
    { return get_pool_vector<T>()->template get<T>(); }

    template<typename T, typename ...Args>
    inline PoolID<T> Pool::create(Args... args)
    {
#ifdef TOOLS_NO_POOL
        T* instance = new T(args...);
        PoolID<T> id{(u64_t)instance};
        instance->poolid( id );
        return id;
#else
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        auto*  vec   = find_or_init_pool_vector<T>();
        size_t index = vec->size();
        T*     data  = &vec->template emplace_back<T>(args...);
        PoolID<T> id = make_record(data, vec, index );
        return id;
#endif
    }

    template<typename T>
    inline PoolID<T> Pool::create()
    {
#ifdef TOOLS_NO_POOL
        T* instance = new T();
        PoolID<T> id{(u64_t)instance};
        instance->poolid( id );
        return id;
#else
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        IPoolVector* pool_vector = find_or_init_pool_vector<T>();
        T* data = &pool_vector->template emplace_back<T>();
        PoolID<T> id = make_record(data, pool_vector, pool_vector->size()-1 );
        return id;
#endif
    }

    template<typename T>
    inline IPoolVector* Pool::init_for()
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        LOG_VERBOSE("Pool", "init_for<%s>() ...\n", tools::type::get<T>()->get_name() );
        auto type_id = std::type_index(typeid(T));
        ASSERT( m_pool_vector_by_type.find(type_id) == m_pool_vector_by_type.end() );
        IPoolVector* new_pool_vector = new TPoolVector<T>( m_config.reserved_size );
        m_pool_vector_by_type.emplace(type_id, new_pool_vector );
        return new_pool_vector;
    }

    template<typename T>
    inline IPoolVector * Pool::get_pool_vector()
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        return m_pool_vector_by_type[typeid(T)];
    }

    template<typename T>
    inline IPoolVector * Pool::find_or_init_pool_vector()
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        auto type_id = std::type_index(typeid(T));
        // TODO: use operator[] instead of find() and force user to call init_for<T>() manually
        auto it = m_pool_vector_by_type.find( type_id );
        if ( it == m_pool_vector_by_type.end() )
        {
            LOG_VERBOSE("Pool", "No vector found for '%s'\n", tools::type::get<T>()->get_name() );
            // Not great to do the init here, but required when a type is not handled yet
            return init_for<T>();
        }
        return it->second;
    }

    template<typename T>
    inline PoolID<T> Pool::make_record(T* data, IPoolVector * vec, size_t pos )
    {
        u32_t next_id = generate_id();
        ASSERT(next_id < invalid_id<u32_t>) // Last id is reserved for "null" or "invalid"
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
    inline void Pool::destroy(PoolID<T> _id )
    {
        STATIC_ASSERT__IS_POOL_REGISTRABLE(T)
        Record& record_to_delete = m_record_by_id[(u64_t)_id];
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

        if( m_config.reuse_ids )
        {
            // Update the "free ids" linked-list
            record_to_delete.next_id = m_first_free_id;
            m_first_free_id = (u64_t)_id;
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
} // namespace tools