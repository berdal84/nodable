#pragma once

#include <cassert>
#include <cstring> // for memcpy
#include <cstdio>
#include <utility> // std::move

namespace tools
{
    enum class Allocation_Strategy // TODO: remove this enum and use allocators!
    {  
        NEXT_ALLOC_USE_HEAP, // Currently allocated on stack, next alloc will be on the heap
        HEAP                 // Currently allocated on the heap, next alloc will be on the heap
    };

    struct String
    {
        using Char_Type = char; // We'll see later to handle different sizes...

        Char_Type*          buf;                    // Pointer to the buffer (static or dynamic)
        size_t              size;                   // String length (excluding terminal string)
        size_t              capacity;               // Buffer size - 1
        Allocation_Strategy allocation_strategy;

        String()
        : allocation_strategy(Allocation_Strategy::HEAP)
        , capacity(0)
        , size(0)
        , buf(nullptr)
        {}

        explicit String(const Char_Type* str)
        : allocation_strategy(Allocation_Strategy::HEAP)
        , size(strlen(str))
        , capacity(0)
        , buf(nullptr)
        {
            if( size > 0 )
            {
                buf = expand_capacity_to_fit(size);
                memcpy(buf, str, size);
                buf[size] = 0;
            }
        }

        String(const String& other)
        : String(other.c_str())
        {}

        String& operator=(const String& other)
        {
            if( this == &other)
                return *this;

            if( capacity < other.capacity)
                buf = expand_capacity_to_fit(other.size);

            memcpy(buf, other.buf, other.size);
            size        = other.size;
            capacity    = other.capacity;
            buf[size]   = 0;

            return *this;
        }

        String(
            Char_Type* data,
            size_t     capacity,
            size_t     length,
            Allocation_Strategy strategy = Allocation_Strategy::NEXT_ALLOC_USE_HEAP
        )
        : allocation_strategy(strategy)
        , capacity(capacity )
        , size(length)
        , buf(data)
        {}


        ~String()
        {
            // TODO: use an allocator
            if( allocation_strategy == Allocation_Strategy::HEAP && buf != nullptr)
            {
                delete[] buf;
            }
        }

        inline const Char_Type* c_str() const
        { return buf != nullptr ? const_cast<const char*>(buf) : ""; }

        inline bool heap_allocated() const
        { return allocation_strategy == Allocation_Strategy::HEAP; }

        /**
        * Expand the buffer to the closest power of two of the desired set_size.
        */
        Char_Type* expand_capacity_to_fit(size_t desired_capacity)
        {
            assert(desired_capacity > capacity );

            // compute the next highest power of 2 of 64-bit

            // based on https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
            size_t new_buf_size = desired_capacity;
            //new_buf_size--; // capacity is buffer_size - 1
            new_buf_size |= new_buf_size >> 1;
            new_buf_size |= new_buf_size >> 2;
            new_buf_size |= new_buf_size >> 4;
            new_buf_size |= new_buf_size >> 8;
            new_buf_size |= new_buf_size >> 16;

            new_buf_size++;

            Char_Type* new_ptr = new Char_Type[new_buf_size];

            if( buf )
            {
                memcpy(new_ptr, buf, size+1); // We only copy the string + null char
            }

            if (allocation_strategy == Allocation_Strategy::HEAP)
            {
                delete[] buf;
            }
            else
            {
                allocation_strategy = Allocation_Strategy::HEAP;
            }
            capacity = new_buf_size - 1;

            return new_ptr;
        }
    };

    inline String* string_append(String* str, const char* buf, size_t length)
    {
        if( str->capacity < str->size + length )
        {
            str->buf = str->expand_capacity_to_fit(str->size + length);
        }
        memcpy(str->buf + str->size, buf, length);
        str->size += length;
        str->buf[str->size] = 0;
        return str;
    }

    inline String* string_append(String* str, const String* other)
    {
        return string_append(str, other->buf, other->size);
    }

    inline String* string_append(String* str, const char* c_str)
    {
        return string_append(str, c_str, strlen(c_str));
    }

    template<typename ...Args>
    size_t string_append_fmt(String* str, const char* format, Args...args )
    {
        return str->size = snprintf(str->buf+str->size, str->capacity-str->size, format, args... );
    }

    template<typename ...Args>
    size_t string_append_fmt(String* str, const char* _str )
    {
        return str->size = snprintf(str->buf+str->size, str->capacity-str->size, "%s", _str );
    }

    /** provided to easily switch to/from std::string */
    inline String* string_push_back(String* str, String::Char_Type c)
    {
        return string_append(str, &c, 1);
    }

    inline bool string_is_empty(const String* str)
    {
        return str->size == 0;
    }
    
    template<typename Char_Type>
    void string_clear(String* str)
    {
        str->size = 0;

        if( str->buf != nullptr)
        {
            str->buf[0] = 0;
        }
    }

    template<typename Char_Type>
    bool operator==(const String& left, const String& right)
    {
        return left.size == right.size
            && strcmp(left.c_str(), right.c_str()) == 0;
    }

    /**
     * Stack allocated string.
     * Switches to dynamic allocations when stack buffer is too small.
     *
     * Buffer set_size and string length are stored in an unsigned integer (1 byte)
     */
    template<size_t STATIC_BUF_SIZE>
    struct Inline_String : public String
    {
        using Char_Type = char;

        static_assert(STATIC_BUF_SIZE >= 8);
        Char_Type static_buf[STATIC_BUF_SIZE]; // Static buffer

        Inline_String()
        : String(static_buf, STATIC_BUF_SIZE-1, 0)
        { static_buf[0] = '\0'; }

        Inline_String(Char_Type *str, size_t length)
        : String(static_buf, STATIC_BUF_SIZE-1, 0)
        { string_append(this, str, length); }

        Inline_String(const Char_Type *str)
        : Inline_String(const_cast<char*>(str), strlen(str))
        {}

        Inline_String(const String& other)
        : Inline_String()
        { string_append(this, other.buf, other.size ); }
    };

    // Define some aliases
    using String_8   = Inline_String<8>;
    using String_16  = Inline_String<16>;
    using String_32  = Inline_String<32>;
    using String_64  = Inline_String<64>;
    using String_128 = Inline_String<128>;
    using String_256 = Inline_String<256>;
    using String_512 = Inline_String<512>;
}