#pragma once
#include "Types.hpp"
#include "String_Hash.hpp"
#include "Type_Traits.hpp"

#define HASHMAP_WALK(it, hash_map) \
for(u32_t i = 0; i < (hash_map).entries.size; ++i ) \
{ \
    auto& it = (hash_map).entries[i]; \
    if( it.state == 0) continue; \
    
#define HASHMAP_WALK_END \
}

namespace bdc
{
    template <typename T, typename Pointer_T = std::remove_cv_t<std::remove_pointer_t<T>>*>
    auto as_pointer(T& t) -> Pointer_T
    {
        if constexpr (std::is_pointer_v<T>)
        {
            return const_cast<Pointer_T>(t);
        }
        else
        {
            return const_cast<Pointer_T>(&t);
        }
    }
    
    // User must implement this template with his own types
    template<typename Key_Type, typename Hash_Type>
    Hash_Type hash(const Key_Type& key)
    {
        static_assert(false, "No predefined implementation of this hash function, define yours");
    };

    // String implemenentation
    template<>
    inline u32_t hash(const String& key)
    {
        return string_hash(key).hash;
    };

    // String_Hash implemenentation
    template<>
    inline u32_t hash(const String_Hash& key)
    {
        return key.hash;
    };

    enum Hash_Map_Slot_State
    {
        Hash_Map_Slot_State_FREE      = 0,
        Hash_Map_Slot_State_OCCUPIED  = 1,
        Hash_Map_Slot_State_REMOVED   = 2
    };

    template<
        typename _Key_Type,
        typename _Value_Type,
        typename _Hash_Type  = u32_t
    >
    struct Hash_Map
    {
        using Key_Type          = _Key_Type;
        using Value_Type        = _Value_Type;
        using Hash_Type         = _Hash_Type;
        using Hash_Proc_Type    = Hash_Type(*)(const Key_Type&);

        struct Entry_Type
        {
            Hash_Type  hash;
            Key_Type   key;
            Value_Type value;
            Hash_Map_Slot_State state; // TODO: once implem works well, get rid of this and use zero initialized key instead
        };

        u32_t                           size;
        u32_t                           capacity;
        Resizable_Array<Entry_Type>     entries;
        Allocator*                      allocator;
        Hash_Proc_Type                  hash_proc;
    };

    // Some templates to deduce if a type is an Hash_Map and get its sub types
    // template<typename T>
    // concept Is_Hash_Map = requires(T& t) {
    //     []<typename K, typename V, typename H>(Hash_Map<K, V, H, hashmap_hash<K, H>>&){}(t);
    // };    
    template<typename T> struct Is_Hash_Map_Implem : std::false_type {};
    template<typename Key_Type, typename Value_Type, typename Hash_Type>
    struct Is_Hash_Map_Implem< Hash_Map<Key_Type, Value_Type, Hash_Type> > : std::true_type {};

    template<typename T>
    concept Is_Hash_Map = Is_Hash_Map_Implem<T>::value;

    void _hashmap_resize_entries_to_optimal_size(Is_Hash_Map auto& hashmap, u32_t capacity_min = 16);

    template<Is_Hash_Map T>
    void hashmap_init(
        T&         hashmap,
        u32_t      initial_capacity  = 16,
        Allocator* _allocator        = nullptr,
        typename T::Hash_Proc_Type hash_proc = &hash<typename T::Key_Type, typename T::Hash_Type>)
    {
        hashmap.allocator = _allocator ? _allocator : allocator;
        hashmap.size      = 0;        
        hashmap.hash_proc = hash_proc;
        hashmap.capacity  = initial_capacity;

        array_init(hashmap.entries, initial_capacity, hashmap.allocator);
        array_resize(hashmap.entries, initial_capacity);
    };

    void hashmap_release(Is_Hash_Map auto& hashmap )
    {
        array_release(hashmap.entries);
        hashmap.entries  = {};
        hashmap.size     = 0;
        hashmap.capacity = 0;
    };

    template<typename Value_Type>
    struct Result
    {
        bool            ok;
        Value_Type      value;
        inline operator bool () const { return ok; }
    };

    template<>
    struct Result<void>
    {
        bool            ok;
        inline operator bool () const { return ok; }
    };

    template<Is_Hash_Map T>
    Result<std::remove_pointer_t<typename T::Entry_Type>*>
    hashmap_add(T& hashmap, const typename T::Key_Type& key, const typename T::Value_Type& value)
    {
        _hashmap_resize_entries_to_optimal_size(hashmap);

        // TODO:
        // - check if entries needs to be resized
        // - resize or not
        // - hash the key
        auto hash = hashmap.hash_proc(key);

        // Get the ideal index in hashmap.entries that correspond to this hash
        auto index = (hash & (hashmap.entries.size - 1) ); // cheap modulo

        // Ensure there are no duplicates
        u32_t iteration = 0;
        while( true )
        {
            assert(iteration < hashmap.entries.size / 4 * 3 && "Too much iterations for hashmap_add, it means we found too much collisions.");
            auto& entry = hashmap.entries[index];

            if( entry.state != Hash_Map_Slot_State_OCCUPIED )
            {
                entry.hash  = hash;
                entry.key   = key;
                entry.value = value;
                entry.state = Hash_Map_Slot_State_OCCUPIED;

                hashmap.size += 1;

                return { .ok = true, .value = as_pointer(entry) };
            }

            if( entry.hash == hash && entry.key == key ) // duplicate found!
            {
                return { .ok = false, .value = nullptr };
            }

            // Here we must try a different index because the selected one is not available to us...
            // The basic strategy I use is simply to increment the index
            index = (index + 1) & (hashmap.entries.size - 1);
            ++iteration;
        }
    };

    template<Is_Hash_Map T>
    Result<u32_t> hashmap_find_index(const T& hashmap, const typename T::Key_Type& key)
    {
        if( hashmap.size == 0) return { .ok = false };

        u32_t hash      = hashmap.hash_proc(key);
        u32_t index     = (hash & (hashmap.entries.size - 1) ); // cheap modulo
        u32_t iteration = 0;

        while( true )
        {
            assert(iteration < hashmap.entries.size / 4 * 3 && "Too much iterations for hashmap_find_index, it means we found too much collisions.");
            auto& entry = hashmap.entries[index];

            // If we hit a non occupied slot, we consider the search finished
            if ( entry.state == Hash_Map_Slot_State_FREE )
            {
                return { .ok = false };
            }

            if( entry.state == Hash_Map_Slot_State_OCCUPIED && entry.hash == hash && entry.key == key ) // two keys might have the same hash, we must compare key after hash.
            {
                return { .ok = true, .value = index };
            }

            index = (index + 1) & (hashmap.entries.size - 1);
            ++iteration;
        }
    };

    template<Is_Hash_Map T>
    Result<std::remove_pointer_t<typename T::Value_Type>*>
    hashmap_remove(T& hashmap, const typename T::Key_Type& key)
    {
        Result<u32_t> found = hashmap_find_index(hashmap, key);
        if( found.ok )
        {
            hashmap.entries[found.value].state = Hash_Map_Slot_State_REMOVED;
            return { .ok = true, .value = as_pointer(hashmap.entries[found.value].value) };
        }
        return { .ok = false };
    };

    template<Is_Hash_Map T>
    Result<std::remove_pointer_t<typename T::Value_Type>*>
    hashmap_find(const T& hashmap, const typename T::Key_Type& key)
    {
        Result<u32_t> found = hashmap_find_index(hashmap, key);
        if( found.ok )
        {
            return { .ok = true, .value = as_pointer(hashmap.entries[found.value].value) };
        }
        return { .ok = false, .value = nullptr };
    };

    template<Is_Hash_Map Hash_Map_Type>
    void _hashmap_resize_entries_to_optimal_size(Hash_Map_Type& hashmap, u32_t capacity_min)
    {
        // Reallocate a larger buffer when size is above 75% capacity or capacity is bellow minimum
        if ( hashmap.size > hashmap.capacity / 4 * 3 || hashmap.capacity < capacity_min)
        {
            assert(hashmap.capacity <= ((u32_t)-1) / 2);
            u32_t optimal_size = hashmap.capacity * 2; // exponential grow

            //
            // TODO: Instead of running this copy at the time user insert a new element,
            //       we can do this piece by piece (ex: 10 items at a time).
            //       A method exist for that and requires to store the latest copied index.
            //       With that method, each time you run an operation on the hashmap, you copie a chunk
            //       after few calls, all the old entries are rehashed and copied to the new data.
            //

            // Allocate a new array for the entries
            Hash_Map_Type new_hashmap{};
            hashmap_init(new_hashmap, optimal_size, hashmap.allocator);

            // Rehash current hashmap entries into the new one
            HASHMAP_WALK(it, hashmap)
            {
                hashmap_add(new_hashmap, it.key, it.value);
            }
            HASHMAP_WALK_END

            // Release old hashmap memory and replace with the new one
            hashmap_release(hashmap);
            hashmap = new_hashmap;
        }
    };
}
