#pragma once

#if !TOOLS_POOL_ENABLE

#define POOL_REGISTRABLE( Class ) /* TOOLS_POOL_ENABLE is OFF */

#else

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
    template<typename T> using PoolID = ::tools::T>; \
    Class> m_id; \
public: \
    using is_pool_registrable = std::true_type; \
    Class> poolid() const { return m_id; }; \
    void poolid(Class> _id) { m_id = _id; }

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
     * Type> id = ...;  // we assume we have a valid id
     * id->do_something(); // de-reference from a pool id (cost a bit more) + call a method on the pointer.
     *
     * Type* ptr = id.get(); // de-referencing once before doing lots of calls is faster.
     * ptr->do_this();
     * ...
     * ptr->do_that();
     */
    template<typename T = char>
    class PoolID
    {
    public:
        using id_t = typename ID64<T>::id_t; // for extern use only
        static const PoolID<T> null{};

        constexpr PoolID() = default;
        explicit constexpr PoolID(u64_t _id);
        explicit constexpr PoolID(const ID<T>& _id);

        template<typename OtherT>
        PoolID(const PoolID<OtherT>& other)
        : m_id(other.id())
        {}

        T* get() const; // Return a pointer to the data from the Pool having an id == this->id
        void reset();
        explicit operator bool () const;
        explicit operator u64_t () const;
        PoolID<T>& operator=(PoolID<T> other);
        bool operator==(const PoolID<T>& other) const;
        bool operator!=(const PoolID<T>& other) const;
        T* operator -> ();
        T* operator -> () const;
        T& operator *  ();
        T& operator *  () const;
        ID64<T> id() const { return m_id; }
    private:
        ID64<T> m_id;
    };

    constexpr size_t INVALID_VEC_POS = ~0;

    /**
     * Interface for any PoolVector
     */
    class IPoolVector
    {
    public:
        using index_t = size_t; // for extern use only
        static constexpr size_t invalid_index = ~0;
        IPoolVector(void* _data_ptr, size_t _elem_size, std::type_index _type_index);
        virtual ~IPoolVector() {};
        virtual size_t size() const = 0;
        virtual void   pop_back() = 0;
        virtual void   swap(size_t, size_t) = 0;
        virtual u64_t poolid_at( size_t _pos ) const = 0;
        std::type_index type_index() const;
        const char* type_name() const;
        void* operator[](size_t _pos) const;
        template<class T, typename ...Args> T& emplace_back(Args ...args);
        template<class T> T& emplace_back();
        template<class T> std::vector<T>& get();
        template<class T> const std::vector<T>& get() const;
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
        IPoolVector*         vector{ nullptr};
        IPoolVector::index_t pos{IPoolVector::invalid_index}; // 0-based
        PoolID<>::id_t       next_id{};
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
            bool       reuse_ids{true}; // for debugging purposes
            size_t     reserved_size{0};
        };

        explicit Pool(const Config&);
        ~Pool();

        template<typename T>          IPoolVector* init_for();
        template<typename T>          T* get(u64_t id);
        template<typename T>          T* get(PoolID<T> _id);
        template<typename T>          void get(std::vector<T*>& _out, const std::vector<PoolID<T>>& _ids);
        template<typename T>          std::vector<T*> get(const std::vector<PoolID<T>>& _ids);
        template<typename T>          std::vector<T>& get_all();
        template<typename T, typename ...Args> PoolID<T> create(Args... args);
        template<typename T>          PoolID<T> create();
        template<typename T>          void destroy(PoolID<T> _id );
        template<typename ContainerT> void destroy_all(const ContainerT& ids);
    private:

        PoolID<>::id_t generate_id();

        template<typename T>          PoolID<T>    make_record(T* data, IPoolVector * vec, size_t pos );
        template<typename T>          IPoolVector* get_pool_vector();
        template<typename T>          IPoolVector* find_or_init_pool_vector(); // prefer get_pool_vector if you are sure it exists

        const Config                                      m_config;
        PoolID<>::id_t                                    m_first_free_id; // Linked-list of free ids
        std::vector<Record>                               m_record_by_id;
        std::unordered_map<std::type_index, IPoolVector*> m_pool_vector_by_type;
    };
} // namespace tools
#endif