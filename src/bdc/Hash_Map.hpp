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

    template<typename Value_Type>
    struct Result
    {
        using Value_Type_Ptr = std::remove_pointer_t<Value_Type>*;

        bool            ok;
        Value_Type_Ptr  value;
        inline operator bool () const { return ok; }
    };

    template<typename Value_Type>
    inline bool operator==(const Result<Value_Type>& result, const Value_Type& value)
    {
        return result.ok ? *result.value == value : false;
    }

    enum Hash_Map_Slot_State
    {
        Hash_Map_Slot_State_FREE      = 0,
        Hash_Map_Slot_State_OCCUPIED  = 1
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

    template<Is_Hash_Map T> using Hash_Map_Key      = typename T::Key_Type;
    template<Is_Hash_Map T> using Hash_Map_Value    = typename T::Value_Type;
    template<Is_Hash_Map T> using Hash_Map_Hash     = typename T::Hash_Type;

    void _hashmap_resize_entries_to_optimal_size(Is_Hash_Map auto& hashmap);

    template<Is_Hash_Map T>
    void hashmap_init(
        T& hashmap, 
        Allocator* allocator = default_allocator(),
        typename T::Hash_Proc_Type hash_proc = &hash<Hash_Map_Key<T>, Hash_Map_Hash<T>>)
    {
        hashmap.allocator = allocator;
        hashmap.size      = 0;
        hashmap.capacity  = 0;
        hashmap.hash_proc = hash_proc;

        _hashmap_resize_entries_to_optimal_size(hashmap);
    };

    void hashmap_release(Is_Hash_Map auto& hashmap )
    {
        array_release(hashmap.entries);
        hashmap.entries = {};
    };

    template<Is_Hash_Map T>
    Result<typename T::Entry_Type> hashmap_add(T& hashmap, const Hash_Map_Key<T>& key, const Hash_Map_Value<T>& value)
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
        while( true )
        {
            auto& entry = hashmap.entries[index];

            if( entry.state == Hash_Map_Slot_State_FREE )
            {
                break;
            }

            if( entry.hash == hash && entry.key == key ) // duplicate found!
            {
                return { .ok = false, .value = nullptr };
            }

            // Here we must try a different index because the selected one is not available to us...
            // The basic strategy I use is simply to increment the index
            index = (index + 1) & (hashmap.entries.size - 1);
        }

        // Write Entry at given location
        hashmap.entries[index].hash     = hash;
        hashmap.entries[index].key      = key;
        hashmap.entries[index].value    = value;
        hashmap.entries[index].state    = Hash_Map_Slot_State_OCCUPIED;

        hashmap.size += 1;

        return { .ok = true, .value = as_pointer(hashmap.entries[index]) };
    };

    template<Is_Hash_Map T>
    Result<void> hashmap_remove(T& hashmap, const Hash_Map_Key<T>& key)
    {
        if( hashmap.size == 0) return { .ok = false };

        auto hash  = hashmap.hash_proc(key);
        auto index = (hash & (hashmap.entries.size - 1) ); // cheap modulo

        while( index < hashmap.entries.size )
        {
            auto& entry = hashmap.entries[index];

            if( entry.hash == hash && entry.key == key ) // two keys might have the same hash, we must compare key after hash.
            {
                entry = {};
                hashmap.size -= 1;
                return { .ok = true };
            }

            ++index; // continue to search...
        }
        return { .ok = false };
    };

    template<Is_Hash_Map T, typename Return_Type = Result<typename T::Value_Type>>
    Return_Type hashmap_find(const T& hashmap, const Hash_Map_Key<T>& key)
    {
        auto hash = hashmap.hash_proc(key);

        for(u32_t i = 0; i < hashmap.entries.size; ++i )
        {
            auto& entry = hashmap.entries[i];
            if( entry.hash == hash && entry.key == key )
            {
                return { .ok = true, .value = as_pointer(entry.value)};
            }
        }

        return { .ok = false, .value = nullptr };
    };

    void _hashmap_resize_entries_to_optimal_size(Is_Hash_Map auto& hashmap)
    {
        // By default, we init entries with a default size
        if( hashmap.capacity == 0 )
        {
            assert(hashmap.size == 0 && "Should be zero if capacity is");
            array_init(hashmap.entries, 0, hashmap.allocator);
            array_resize(hashmap.entries, 16);
            hashmap.capacity = hashmap.entries.capacity;
            hashmap.size     = 0;
            return;
        }

        // Reallocate a larger buffer when size is above 75% capacity
        u32_t ideal_size = 3 * (hashmap.capacity / 4);
        if ( hashmap.size > ideal_size )
        {
            assert(hashmap.capacity <= ((u32_t)-1) / 2);
            u32_t optimal_buffer_size = hashmap.capacity * 2; // exponential grow
            array_resize( hashmap.entries, optimal_buffer_size );
            hashmap.capacity = hashmap.entries.capacity;
        }
    };
}
