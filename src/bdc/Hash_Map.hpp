#pragma once
#include "Types.hpp"
#include "String_Hash.hpp"
#include "Type_Traits.hpp"

#define HASHMAP_WALK(it, hash_map) \
for(u32_t i = 0; i < (hash_map).entries.size; ++i ) \
{ \
    auto& it = (hash_map).entries[i]; \
    if( it.hash < Hash_Map_Hash_OCCUPIED_RANGE_START ) continue; \
    
#define HASHMAP_WALK_END \
}

namespace bdc
{
    enum Hash_Map_Hash
    {
        Hash_Map_Hash_FREE                  = 0,
        Hash_Map_Hash_TOMBSTONE             = 1,
        Hash_Map_Hash_OCCUPIED_RANGE_START
    };

    template <
        typename Type,
        typename Pointer_Type = std::remove_cv_t<std::remove_pointer_t<Type>>*
    >
    Pointer_Type as_pointer(Type& t)
    {
        if constexpr (std::is_pointer_v<Type>)
            return const_cast<Pointer_Type>(t);
        else
            return const_cast<Pointer_Type>(&t);
    }
    
    // User must implement this template with his own types
    template<typename Key_Type, typename Hash_Type>
    Hash_Type hash(const Key_Type& key)
    {
        if constexpr ( std::is_integral_v<Key_Type> )
        {
            return (Hash_Type)key; // it's ok if we truncate keys larger than Hash_Type, our Hash_Map cannot contain more than 2^32 elements.
        }
        else if constexpr ( std::is_same_v<Key_Type, String_Hash> )
        {
            return key.hash;
        }
        else if constexpr ( std::is_same_v<Key_Type, String> )
        {
            return string_hash(key).hash;
        }
        else
        {
            static_assert(false, "This Key_Type is not handled by default, please implement Hash_Type hash(const Key_Type& key)");
        }
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
            Hash_Type  hash; // HASHMAP_HASH_FREE and HASHMAP_HASH_TOMBSTONE are reserved, other values are considered OCCUPIED.
            Key_Type   key;
            Value_Type value;
        };

        u32_t                           size      = 0;      // occupied
        u32_t                           live_size = 0; // occupied + tombstones
        u32_t                           capacity  = 0;  // occupied + tombstones + never used
        Resizable_Array<Entry_Type>     entries   = {};
        Allocator*                      allocator = nullptr;
        Hash_Proc_Type                  hash_proc = nullptr;
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

    void  hashmap_resize_entries_to_optimal_size(Is_Hash_Map auto& hashmap, u32_t capacity_min = 16);
    
    template<Is_Hash_Map Hash_Map_Type>
    Hash_Map_Type::Hash_Type hashmap_index(const Hash_Map_Type& hashmap, typename Hash_Map_Type::Hash_Type desired_index)
    {
        //
        // We also apply a cheap modulo ( hash & ... ) to ensure values stays in entries range.
        // This makes different hash to have the same index, the user of this function has to deal with that.
        // The implementation of Hash_Map simply take the next FREE or TOMBSTONE slot. Once the table is rehashed,
        // the tombstones are dropped.
        //
        typename Hash_Map_Type::Hash_Type index = desired_index & (hashmap.entries.size - 1);
        //
        // To avoid storing a state in each Entry (FREE, OCCUPIED, TOMBSTONE) we reserve 2 values.
        // - HASHMAP_HASH_FREE:       was never OCCUPIED.
        // - HASHMAP_HASH_TOMBSTONE:  was OCCUPIED.
        // By consequences, any other value is considered OCCUPIED
        //
        if (index < Hash_Map_Hash_OCCUPIED_RANGE_START)
            return Hash_Map_Hash_OCCUPIED_RANGE_START;
        return index;
    }

    template<Is_Hash_Map T>
    void hashmap_init(
        T&         hashmap,
        u32_t      initial_capacity  = 16,
        Allocator* _allocator        = nullptr,
        typename T::Hash_Proc_Type hash_proc = &hash<typename T::Key_Type, typename T::Hash_Type>)
    {
        hashmap.allocator = _allocator ? _allocator : allocator;
        hashmap.size      = 0;        
        hashmap.live_size = 0;
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
        hashmap.live_size= 0;
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
        hashmap_resize_entries_to_optimal_size(hashmap);

        // TODO:
        // - check if entries needs to be resized
        // - resize or not
        // - hash the key
        auto hash = hashmap.hash_proc(key);

        // Get the ideal index in hashmap.entries that correspond to this hash
        auto index = hashmap_index(hashmap, hash);

        // Ensure there are no duplicates
        u32_t iteration = 0;
        while( true )
        {
            assert(iteration < hashmap.entries.size / 4 * 3 && "Too much iterations for hashmap_add, it means we found too much collisions.");
            auto& entry = hashmap.entries[index];

            if( entry.hash < Hash_Map_Hash_OCCUPIED_RANGE_START )
            {
                entry.hash  = hash;
                entry.key   = key;
                entry.value = value;

                hashmap.size      += 1;
                hashmap.live_size += 1;

                return { .ok = true, .value = as_pointer(entry) };
            }

            if( entry.hash == hash && entry.key == key ) // duplicate found!
            {
                return { .ok = false, .value = nullptr };
            }

            // Here we must try a different index because the selected one is not available to us...
            // The basic strategy I use is simply to increment the index
            index = hashmap_index(hashmap, index + 1);
            ++iteration;
        }
    };

    template<Is_Hash_Map T>
    Result<u32_t> hashmap_find_index(const T& hashmap, const typename T::Key_Type& key)
    {
        if( hashmap.size == 0) return { .ok = false };

        u32_t hash      = hashmap.hash_proc(key);
        u32_t index     = hashmap_index(hashmap, hash);
        u32_t iteration = 0;

        while( true )
        {
            assert(iteration <= hashmap.live_size && "Too much iterations for hashmap_find_index, it means we found too much collisions.");
            auto& entry = hashmap.entries[index];

            // If we hit a non occupied slot, we consider the search finished
            // When it is a tombstone we have to continue, since at insertion time, that slot was perhaps occupied
            // and by consequences the next slot might have been used.
            if ( entry.hash == Hash_Map_Hash_FREE )
            {
                return { .ok = false };
            }

            if( entry.hash != Hash_Map_Hash_TOMBSTONE && entry.hash == hash && entry.key == key ) // two keys might have the same hash, we must compare key after hash.
            {
                return { .ok = true, .value = index };
            }

            index = hashmap_index(hashmap, index + 1);
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
            assert(hashmap.size != 0);
            hashmap.entries[found.value].hash = Hash_Map_Hash_TOMBSTONE;
            hashmap.size -= 1;
            // hashmap.live_size remains the same, because having tombstones has an effect when removing elements
            // indeed, it takes more time when the map is full of tombstones.
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
    void hashmap_resize_entries_to_optimal_size(Hash_Map_Type& hashmap, u32_t capacity_min)
    {
        const u32_t critical_size = hashmap.capacity / 3 * 2;

        // Reallocate a larger buffer when size is above 66% capacity or capacity is bellow minimum
        if ( hashmap.live_size > critical_size || hashmap.capacity < capacity_min)
        {
            assert(hashmap.capacity <= ((u32_t)-1) / 2);
            u32_t new_capacity = hashmap.capacity;

            if( new_capacity < capacity_min )
            {
                new_capacity = capacity_min;
            }
            else if( hashmap.size > critical_size )
            {
                //
                // We only double the capacity if the size (not live_size) is greater than the critical size.
                // Because when re-hashing, we'll drop the tombstones, so the size that matters it size, not live_size.
                //
                new_capacity *= 2;
            }
            

            //
            // TODO: Instead of running this copy and re-hash fully when the user insert a new element,
            //       we could do this chunk by chunk (ex: 10 elements at a time).
            //       A method exist for that and requires to store the a cursor to the last re-hashed and copied index.
            //       With that method, each time you run an operation on the hashmap, you copy/re-hash a chunk
            //       after few calls, all the old entries are rehashed and copied to the new data and old data can be destroyed.
            //

            // Allocate a new array for the entries
            Hash_Map_Type new_hashmap{};
            hashmap_init(new_hashmap, new_capacity, hashmap.allocator);

            // Rehash current hashmap entries into the new one
            HASHMAP_WALK(it, hashmap)
                hashmap_add(new_hashmap, it.key, it.value);
            HASHMAP_WALK_END

            assert( new_hashmap.size == new_hashmap.live_size && "New Hash_Map should have no tombstone!");

            // Release old hashmap memory and replace with the new one
            hashmap_release(hashmap);
            hashmap = new_hashmap;
        }
    };
}
