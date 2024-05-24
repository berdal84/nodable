#pragma once

#include <algorithm>
#include <cstdlib>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <functional>

#include "tools/core/TIdentifier.h"
#include "tools/core/assertions.h"

#ifdef TOOLS_DEBUG
#include "tools/core/reflection/type.h"
#endif

#define POOL_REGISTRABLE( Class ) \
protected: \
    template<typename T> using ID     = ::tools::ID<T>; \
    template<typename T> using PoolID = ::tools::PoolID<T>; \
    PoolID<Class> m_id; \
public: \
    using is_pool_registrable = std::true_type; \
    PoolID<Class> poolid() const { return m_id; }; \
    void poolid(PoolID<Class> _id) { m_id = _id; }

#define STATIC_ASSERT__IS_POOL_REGISTRABLE(T) \
static_assert( std::is_same_v<typename T::is_pool_registrable, std::true_type>, "This type is not pool registrable, use POOL_REGISTRABLE macros." );

namespace tools
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
    template<typename T = void>
    class PoolID
    {
        friend class Pool;
    public:
        static PoolID<T> null;
        ID64<T>          id;

        PoolID() = default;
        explicit PoolID(u64_t _id);
        explicit PoolID(const ID<T>& _id);

        template<typename OtherT>
        PoolID(const PoolID<OtherT>& other)
        : id(other.id)
        {}

        T* get() const; // Return a pointer to the data from the Pool having an id == this->id
        void reset();
        inline explicit operator bool () const;
        inline explicit operator u64_t () const;
        inline PoolID<T>& operator=(PoolID<T> other);
        inline bool operator==(const PoolID<T>& other) const;
        inline bool operator!=(const PoolID<T>& other) const;
        inline T* operator -> ();
        inline T* operator -> () const;
        inline T& operator *  ();
        inline T& operator *  () const;
    };

    /**
     * Interface for any PoolVector
     */
    class IPoolVector
    {
    public:
        inline IPoolVector(void* _data_ptr, size_t _elem_size, std::type_index _type_index);
        virtual ~IPoolVector() {};
        virtual size_t size() const = 0;
        virtual void   pop_back() = 0;
        virtual void   swap(size_t, size_t) = 0;
        virtual u64_t poolid_at( size_t _pos ) const = 0;
        inline std::type_index type_index() const;
        inline const char* type_name() const;
        inline void* operator[](size_t _pos) const;
        template<class T, typename ...Args> inline T& emplace_back(Args ...args);
        template<class T> inline T& emplace_back();
        template<class T> inline std::vector<T>& get();
        template<class T> inline const std::vector<T>& get() const;
    protected:
        void*           m_vector_ptr;
        size_t          m_elem_size;
        std::type_index m_type_index;
    };


    /** Templated implementation of IPoolVector
     * Provide a way to deal with typed vector while respecting the IPoolVector interface */
    template<typename T>
    class TPoolVector : public IPoolVector
    {
    public:
        TPoolVector(size_t _capacity = 0)
            : IPoolVector(&m_vector, sizeof(T), typeid(T))
            , m_vector()
        {
            m_vector.reserve( _capacity );
        }

        ~TPoolVector()
        {};

        TPoolVector(const TPoolVector&) = delete;
        TPoolVector& operator=(const TPoolVector&) = delete;
        TPoolVector(TPoolVector&&) = delete;
        TPoolVector& operator=(TPoolVector&&) = delete;

        void swap( size_t _a, size_t _b ) override
        { std::swap( m_vector.at( _a ), m_vector.at( _b ) ); };

        void pop_back() override
        { m_vector.pop_back(); };

        size_t size() const override
        { return m_vector.size(); };

        u64_t poolid_at(size_t _pos) const override
        { return (u64_t)m_vector[_pos].poolid(); }

    private:
        std::vector<T> m_vector;
    };

    /**
     * When a new instance is created, a new record is added.
     * It allow to find the location of the object in memory and its type.
     */
    struct Record
    {
        IPoolVector* vector{ nullptr};
        size_t       pos{invalid_id<size_t>}; // Zero-based position of the data in the vector.
        u64_t        next_id{invalid_id<u64_t>}; // id to the next Record, if pos is invalid it points to the next free id.
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

        struct Config
        {
            const bool reuse_ids{true}; // for debugging purposes
            size_t     reserved_size{0};
        };

        Pool(const Config&);
        ~Pool();

        template<typename T>          inline IPoolVector* init_for();
        template<typename T>          inline T* get(u64_t id);
        template<typename T>          inline T* get(PoolID<T> _id);
        template<typename T>          inline void get(std::vector<T*>& _out, const std::vector<PoolID<T>>& _ids);
        template<typename T>          inline std::vector<T*> get(const std::vector<PoolID<T>>& _ids);
        template<typename T>          inline std::vector<T>& get_all();
        template<typename T, typename ...Args> inline PoolID<T> create(Args... args);
        template<typename T>          inline PoolID<T> create();
        template<typename T>          inline void destroy(PoolID<T> _id );
        template<typename ContainerT> inline void destroy_all(const ContainerT& ids);
    private:

        inline u64_t generate_id();

        template<typename T>          inline PoolID<T>    make_record(T* data, IPoolVector * vec, size_t pos );
        template<typename T>          inline IPoolVector* get_pool_vector();
        template<typename T>          inline IPoolVector* find_or_init_pool_vector(); // prefer get_pool_vector if you are sure it exists

        const Config                                      m_config;
        u64_t                                             m_first_free_id; // Linked-list of free ids
        std::vector<Record>                               m_record_by_id;
        std::unordered_map<std::type_index, IPoolVector*> m_pool_vector_by_type;
    };

    struct PoolManager
    {
        struct Config
        {
            Pool::Config default_pool_config{};
        };

        Pool* get_pool(); // Right now there is only 1 Pool, but we should split (1 pool per type)
        std::vector<Pool> pools{};
    };


    Pool*        get_pool();
    PoolManager* init_pool_manager(PoolManager::Config = {}); // Call this before to use.
    void         shutdown_pool_manager(); // Undo init_task_manager()

} // namespace tools

#include "Pool.inl"