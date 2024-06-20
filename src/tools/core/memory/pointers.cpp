#include "pointers.h"
#include "tools/core/assertions.h"

namespace tools {

    template<class T> struct ID;
    template<class T> struct Pool;

    template<class T>
    struct Pool
    {
        ID<T> create();
        T*    get_ptr(ID<T> id);

        std::vector<T> instances;
    };

    template<class T>
    struct ID
    {
        ID(const T* raw_ptr)
        : ID(raw_ptr->id)
        { static_assert(std::is_same_v<decltype(raw_ptr->id), ID<T>>, "T must have a field \"ID<T> id\"" );}

        ID(const ID& other)
        : pool(other.pool)
        , id(other.id)
        {}

        ID(Pool<T>* pool, size_t id)
        : id(id)
        {}

        operator T *()
        { return pool->get_ptr(*this); }

        bool operator ==(ID other) const
        { return id == other.id; }

        bool operator !=(ID other) const
        { return id != other.id; }

        // more operators here ...

        size_t id;
    private:
        Pool<T>* pool;
    };

    struct Boat{};
    struct Plane
    {
        ID<Plane> id;
    };

// Custom wrapped pointer type, "type" will be used when use does Ptr<Plane>
    template<>
    struct WrappedPtr<Plane> {
        using type = TWrappedPtr<Plane, ID<Plane> >;
    };

    // Static checks
    static_assert(std::is_same_v<Ptr<Boat>, Boat*> /* default! */ );
    static_assert(!Ptr<Plane>::uses_raw_ptr);
    static_assert(std::is_same_v<Ptr<Plane>, WrappedPtr<Plane>::type> /* user defined! */ );

}

using namespace tools;

void test()
{
    // Usage #1:
    Ptr<Boat> boat = new Boat();
    ASSERT(boat != nullptr)

    // Usage #2:

    Pool<Plane> pool{};
    Ptr<Plane> plane = pool.create();
    ASSERT(plane != nullptr)
}
